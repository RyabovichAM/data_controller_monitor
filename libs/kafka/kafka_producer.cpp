#include "kafka_producer.h"

namespace kafka {

KafkaProducer::KafkaProducer(const std::string& brokers, const std::string& topic)
    : topic_{topic} {
    std::string errstr;
    std::unique_ptr<RdKafka::Conf> conf{
        RdKafka::Conf::create(RdKafka::Conf::CONF_GLOBAL)};

    conf->set("bootstrap.servers", brokers, errstr);
    conf->set("enable.idempotence", "true", errstr);

    producer_.reset(RdKafka::Producer::create(conf.get(), errstr));
    if (!producer_) {
        ReportError(errstr);
    }
}

KafkaProducer::~KafkaProducer() {
    if (producer_) {
        Flush();
    }
}

void KafkaProducer::SetErrorHandler(ProducerErrorHandler handler) {
    error_handler_ = handler;
}

bool KafkaProducer::Publish(const std::string& key, const std::string& payload) {
    if (!producer_) {
        return false;
    }

    RdKafka::ErrorCode err = producer_->produce(
        topic_,
        RdKafka::Topic::PARTITION_UA,
        RdKafka::Producer::RK_MSG_COPY,
        const_cast<char*>(payload.data()), payload.size(),
        key.data(), key.size(),
        0, nullptr);

    if (err != RdKafka::ERR_NO_ERROR) {
        ReportError(RdKafka::err2str(err));
        return false;
    }

    producer_->poll(0);
    return true;
}

void KafkaProducer::Flush(int timeout_ms) {
    producer_->flush(timeout_ms);
}

void KafkaProducer::ReportError(const std::string& message) {
    if (error_handler_) {
        error_handler_(message);
    }
}

}   //kafka
