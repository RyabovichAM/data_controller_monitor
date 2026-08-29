#ifndef KAFKA_PRODUCER_H
#define KAFKA_PRODUCER_H

#include <functional>
#include <memory>
#include <string>

#include <librdkafka/rdkafkacpp.h>

namespace kafka {

using ProducerErrorHandler = std::function<void(const std::string&)>;

class KafkaProducer {
public:
    KafkaProducer(const std::string& brokers, const std::string& topic);
    ~KafkaProducer();

    void SetErrorHandler(ProducerErrorHandler handler);
    bool Publish(const std::string& key, const std::string& payload);
    void Flush(int timeout_ms = 5000);

private:
    std::unique_ptr<RdKafka::Producer> producer_;
    std::string topic_;
    ProducerErrorHandler error_handler_{nullptr};

    void ReportError(const std::string& message);
};

}   //kafka

#endif // KAFKA_PRODUCER_H
