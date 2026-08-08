#include <unordered_map>

#include <QCoreApplication>
#include <QDateTime>
#include <QDir>
#include <QHash>
#include <QJsonDocument>
#include <QTextStream>

#include "data_storage/data_storage_factory.h"
#include "kafka/kafka_consumer.h"

namespace {

using SaveType = QString;
using LoadType = QList<QPair<QDateTime, QJsonDocument>>;
using Storage = data_storage::DataStorageInterface<SaveType, LoadType>;

QTextStream& Log() {
    static QTextStream out{stdout};
    return out;
}

QString Env(const char* name, const QString& fallback = {}) {
    QByteArray value = qgetenv(name);
    return value.isEmpty() ? fallback : QString::fromUtf8(value);
}

class StorageRegistry {
public:
    StorageRegistry(const QString& root, const QString& data_format,
                    const QString& period)
        : root_{root}
        , data_format_{data_format}
        , period_{period} {
    }

    Storage* ForCollector(const QString& collector_id) {
        auto it = storages_.find(collector_id);
        if (it != storages_.end()) {
            return it->second.get();
        }

        QString location = root_ + "/" + collector_id + "/";
        QDir{}.mkpath(location);

        QHash<QString, QString> settings;
        settings["type"] = "file";
        settings["location"] = location;
        settings["data_format"] = data_format_;
        settings["period"] = period_;

        auto storage =
            data_storage::DataStorageFactory::CreateDataStorage<SaveType, LoadType>(settings);
        storage->SetErrorHandler([collector_id](const QString& error) {
            Log() << "[storage:" << collector_id << "] " << error << Qt::endl;
        });
        storage->Open();

        return storages_.emplace(collector_id, std::move(storage)).first->second.get();
    }

private:
    std::unordered_map<QString, std::unique_ptr<Storage>> storages_;
    QString root_;
    QString data_format_;
    QString period_;
};

}   //namespace

int main(int argc, char* argv[]) {
    QCoreApplication qapp{argc, argv};

    const QString brokers = Env("KAFKA_BROKERS", "kafka:9092");
    const QString topic = Env("KAFKA_TOPIC", "sensor-data");
    const QString group_id = Env("KAFKA_GROUP_ID", "storage-service");
    const QString root = Env("STORAGE_ROOT", "/data");
    const QString data_format = Env("STORAGE_FORMAT", "text");
    const QString period = Env("STORAGE_PERIOD_MS", "0");

    StorageRegistry registry{root, data_format, period};

    kafka::KafkaConsumer consumer{brokers, topic, group_id};
    consumer.SetErrorHandler([](const QString& error) {
        Log() << "[kafka] " << error << Qt::endl;
    });
    consumer.SetMessageHandler([&registry](const QString& key, const QByteArray& payload) {
        registry.ForCollector(key)->DataSave(QString::fromUtf8(payload));
    });

    Log() << "[storage-service] " << brokers << " topic=" << topic
          << " root=" << root << Qt::endl;

    consumer.Run();

    return 0;
}
