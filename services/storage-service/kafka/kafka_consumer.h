#ifndef KAFKA_CONSUMER_H
#define KAFKA_CONSUMER_H

#include <functional>
#include <memory>
#include <QString>
#include <librdkafka/rdkafkacpp.h>

namespace kafka {

using ErrorHandler = std::function<void(const QString&)>;
using MessageHandler = std::function<void(const QString& key, const QByteArray& payload)>;

class KafkaConsumer {
public:
    KafkaConsumer(const QString& brokers, const QString& topic, const QString& group_id);
    ~KafkaConsumer();

    void SetErrorHandler(ErrorHandler handler);
    void SetMessageHandler(MessageHandler handler);
    void Run();
    void Stop();

private:
    std::unique_ptr<RdKafka::KafkaConsumer> consumer_;
    ErrorHandler error_handler_{nullptr};
    MessageHandler message_handler_{nullptr};
    bool running_{false};

    void ReportError(const QString& message);
};

}   //kafka

#endif // KAFKA_CONSUMER_H
