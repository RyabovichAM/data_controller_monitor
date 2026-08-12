#include <atomic>
#include <csignal>
#include <cstdlib>
#include <exception>
#include <iostream>
#include <memory>
#include <string>

#include <QCoreApplication>
#include <QJsonDocument>
#include <QMetaObject>
#include <QTimer>

#include "app/monitor_unit.h"
#include "config/config_client.h"
#include "config/config_mapping.h"
#include "kafka/kafka_producer.h"

namespace {

std::atomic_bool stop_requested{false};

void RequestStop(int /*signal*/) {
    stop_requested = true;
}

std::string Env(const char* name, const std::string& fallback) {
    const char* value = std::getenv(name);
    return (value && *value) ? std::string{value} : fallback;
}

class KafkaObserver : public app::MU_ObserverBase {
public:
    explicit KafkaObserver(const std::string& collector_id, QObject* parent = nullptr)
        : app::MU_ObserverBase{parent}
        , collector_id_{QString::fromStdString(collector_id)} {
    }

    // The producer is rebuilt whenever a config changes the broker or the topic,
    // so the observer points at it rather than owning it.
    void SetProducer(kafka::KafkaProducer* producer) {
        producer_ = producer;
    }

    void Update(const QJsonDocument& data) override {
        if (producer_) {
            producer_->Publish(collector_id_, data.toJson(QJsonDocument::Compact));
        }
    }

private:
    kafka::KafkaProducer* producer_{nullptr};
    QString collector_id_;
};

// Everything a config drives. Lives in the Qt thread and is only ever touched
// from it: QTcpServer and QSerialPort belong to the thread that created them,
// while configs arrive on the gRPC client's thread.
class Runtime : public QObject {
public:
    explicit Runtime(const std::string& collector_id)
        : observer_{collector_id} {
        unit_.SetName(QString::fromStdString(collector_id));
        unit_.SetObserver(&observer_);
        unit_.SetErrorHandler([](const QString& error) {
            std::cout << "[transfer] " << error.toStdString() << std::endl;
        });
    }

    void Apply(const dcm::config::v1::CollectorConfig& config);
    void Shutdown();

private:
    app::MonitorUnit unit_;
    KafkaObserver observer_;

    std::unique_ptr<kafka::KafkaProducer> producer_;
    config::KafkaSettings kafka_settings_;
    config::TransferSettings transfer_settings_;
    bool running_{false};

    void RebuildProducer(const config::KafkaSettings& settings);
};

void Runtime::Apply(const dcm::config::v1::CollectorConfig& config) {
    config::TransferSettings transfer_settings;
    config::KafkaSettings kafka_settings;
    try {
        transfer_settings = config::ToTransferSettings(config);
        kafka_settings = config::ToKafkaSettings(config);
    } catch (const std::exception& error) {
        // Nothing has been touched yet, so the collector keeps running on what
        // it already had.
        std::cout << "[collector] version " << config.version() << " ignored: "
                  << error.what() << std::endl;
        return;
    }

    if (!producer_ || kafka_settings != kafka_settings_) {
        RebuildProducer(kafka_settings);
    }

    // Reopening a listening socket drops whatever is connected to it, so an
    // unchanged transport is left alone.
    if (running_ && transfer_settings == transfer_settings_) {
        std::cout << "[collector] version " << config.version()
                  << " applied, transport unchanged" << std::endl;
        return;
    }

    unit_.Stop();
    unit_.SetSettings(config::ToMonitorUnitSettings(transfer_settings));
    transfer_settings_ = transfer_settings;

    try {
        unit_.Start();
        running_ = true;
        std::cout << "[collector] version " << config.version() << " applied: "
                  << transfer_settings["type"] << std::endl;
    } catch (const std::exception& error) {
        // A config the server accepted can still be unusable here — an address
        // no interface has, for instance.
        running_ = false;
        std::cout << "[collector] version " << config.version()
                  << " failed to start: " << error.what() << std::endl;
    }
}

void Runtime::Shutdown() {
    unit_.Stop();
    running_ = false;

    observer_.SetProducer(nullptr);
    if (producer_) {
        producer_->Flush();
    }
}

void Runtime::RebuildProducer(const config::KafkaSettings& settings) {
    observer_.SetProducer(nullptr);
    if (producer_) {
        // Whatever was collected before the switch still belongs in the old topic.
        producer_->Flush();
    }

    producer_ = std::make_unique<kafka::KafkaProducer>(
        QString::fromStdString(settings.brokers), QString::fromStdString(settings.topic));
    producer_->SetErrorHandler([](const QString& error) {
        std::cout << "[kafka] " << error.toStdString() << std::endl;
    });

    observer_.SetProducer(producer_.get());
    kafka_settings_ = settings;

    std::cout << "[collector] kafka " << settings.brokers << " topic=" << settings.topic
              << std::endl;
}

}   //namespace

int main(int argc, char* argv[]) {
    QCoreApplication qapp{argc, argv};

    const std::string collector_id = Env("COLLECTOR_ID", "collector-1");
    const std::string config_address = Env("CONFIG_SERVICE_ADDRESS", "config-service:50051");

    std::signal(SIGINT, RequestStop);
    std::signal(SIGTERM, RequestStop);

    Runtime runtime{collector_id};

    config::ConfigClient client{config_address, collector_id};
    client.SetLogHandler([](const std::string& message) {
        std::cout << "[config] " << message << std::endl;
    });
    client.SetConfigHandler([&runtime](const dcm::config::v1::CollectorConfig& config) {
        // Arrives on the client's thread, applied on the Qt one.
        QMetaObject::invokeMethod(
            &runtime, [&runtime, config]() { runtime.Apply(config); }, Qt::QueuedConnection);
    });

    std::cout << "[collector] " << collector_id << " -> config-service " << config_address
              << std::endl;

    client.Start();

    // A signal handler may only touch the flag; leaving the event loop is the
    // loop's own business.
    QTimer stop_timer;
    QObject::connect(&stop_timer, &QTimer::timeout, &qapp, [&qapp]() {
        if (stop_requested) {
            qapp.quit();
        }
    });
    stop_timer.start(200);

    QObject::connect(&qapp, &QCoreApplication::aboutToQuit, [&client, &runtime]() {
        client.Stop();
        runtime.Shutdown();
    });

    return qapp.exec();
}
