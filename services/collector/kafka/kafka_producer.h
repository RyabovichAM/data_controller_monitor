#ifndef KAFKA_PRODUCER_H
#define KAFKA_PRODUCER_H

#include <memory>
#include <QString>
#include <librdkafka/rdkafkacpp.h>

namespace kafka {

using ErrorHandler = std::function<void(const QString&)>;

class KafkaProducer {
public:
    KafkaProducer(const QString& brokers, const QString& topic);
    ~KafkaProducer();

    void SetErrorHandler(ErrorHandler handler);
    bool Publish(const QString& key, const QByteArray& payload);
    void Flush(int timeout_ms = 5000);

private:
    std::unique_ptr<RdKafka::Producer> producer_;
    QString topic_;
    ErrorHandler error_handler_{nullptr};

    void ReportError(const QString& message);
};

}   //kafka

#endif // KAFKA_PRODUCER_H
