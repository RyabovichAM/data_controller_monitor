#include <QCoreApplication>
#include <QJsonDocument>
#include <QTextStream>

#include "app/monitor_unit.h"
#include "kafka/kafka_producer.h"

namespace {

QTextStream& Log() {
    static QTextStream out{stdout};
    return out;
}

QString Env(const char* name, const QString& fallback = {}) {
    QByteArray value = qgetenv(name);
    return value.isEmpty() ? fallback : QString::fromUtf8(value);
}

app::MonitorUnitSettings BuildSettings() {
    app::MonitorUnitSettings settings;

    QString type = Env("TRANSFER_TYPE", "TCP/IP");
    settings.transfer["type"] = type;

    if (type == "TCP/IP") {
        settings.transfer["host"] = Env("TRANSFER_HOST", "127.0.0.1");
        settings.transfer["port"] = Env("TRANSFER_PORT", "5000");
    } else {
        settings.transfer["port_name"] = Env("SERIAL_PORT_NAME", "/dev/ttyUSB0");
        settings.transfer["baud_rate"] = Env("SERIAL_BAUD_RATE", "9600");
        settings.transfer["data_bits"] = Env("SERIAL_DATA_BITS", "8");
        settings.transfer["parity"] = Env("SERIAL_PARITY", "NoParity");
        settings.transfer["stop_bits"] = Env("SERIAL_STOP_BITS", "OneStop");
        settings.transfer["flow_control"] = Env("SERIAL_FLOW_CONTROL", "NoFlowControl");
    }

    return settings;
}

class KafkaObserver : public app::MU_ObserverBase {
public:
    KafkaObserver(kafka::KafkaProducer& producer, const QString& collector_id,
                  QObject* parent = nullptr)
        : app::MU_ObserverBase{parent}
        , producer_{producer}
        , collector_id_{collector_id} {
    }

    void Update(const QJsonDocument& data) override {
        producer_.Publish(collector_id_, data.toJson(QJsonDocument::Compact));
    }

private:
    kafka::KafkaProducer& producer_;
    QString collector_id_;
};

}   //namespace

int main(int argc, char* argv[]) {
    QCoreApplication qapp{argc, argv};

    const QString collector_id = Env("COLLECTOR_ID", "collector-1");
    const QString brokers = Env("KAFKA_BROKERS", "kafka:9092");
    const QString topic = Env("KAFKA_TOPIC", "sensor-data");

    kafka::KafkaProducer producer{brokers, topic};
    producer.SetErrorHandler([](const QString& error) {
        Log() << "[kafka] " << error << Qt::endl;
    });

    app::MonitorUnit unit{BuildSettings()};
    unit.SetName(collector_id);

    KafkaObserver observer{producer, collector_id};
    unit.SetObserver(&observer);

    Log() << "[collector] " << collector_id << " -> " << brokers
          << " topic=" << topic << Qt::endl;

    unit.Start();

    QObject::connect(&qapp, &QCoreApplication::aboutToQuit, [&unit]() {
        unit.Stop();
    });

    return qapp.exec();
}
