#ifndef KAFKA_CONSUMER_H
#define KAFKA_CONSUMER_H

#include <atomic>
#include <functional>
#include <memory>
#include <string>

#include <librdkafka/rdkafkacpp.h>

namespace kafka {

using ErrorHandler = std::function<void(const std::string&)>;
using MessageHandler = std::function<void(const std::string& key, const std::string& payload)>;

class KafkaConsumer {
public:
    KafkaConsumer(const std::string& brokers, const std::string& topic,
                  const std::string& group_id);
    ~KafkaConsumer();

    void SetErrorHandler(ErrorHandler handler);
    void SetMessageHandler(MessageHandler handler);
    void Run();
    void Stop();

private:
    std::unique_ptr<RdKafka::KafkaConsumer> consumer_;
    ErrorHandler error_handler_{nullptr};
    MessageHandler message_handler_{nullptr};
    // Set from a signal handler, read by the polling loop.
    std::atomic_bool running_{false};

    void ReportError(const std::string& message);
};

}   //kafka

#endif // KAFKA_CONSUMER_H
