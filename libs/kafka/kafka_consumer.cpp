#include "kafka_consumer.h"

namespace kafka {

KafkaConsumer::KafkaConsumer(const std::string& brokers, const std::string& topic,
                             const std::string& group_id) {
    std::string errstr;
    std::unique_ptr<RdKafka::Conf> conf{
        RdKafka::Conf::create(RdKafka::Conf::CONF_GLOBAL)};

    conf->set("bootstrap.servers", brokers, errstr);
    conf->set("group.id", group_id, errstr);
    conf->set("auto.offset.reset", "earliest", errstr);
    conf->set("enable.auto.commit", "false", errstr);

    consumer_.reset(RdKafka::KafkaConsumer::create(conf.get(), errstr));
    if (!consumer_) {
        ReportError(errstr);
        return;
    }

    RdKafka::ErrorCode err = consumer_->subscribe({topic});
    if (err != RdKafka::ERR_NO_ERROR) {
        ReportError(RdKafka::err2str(err));
    }
}

KafkaConsumer::~KafkaConsumer() {
    if (consumer_) {
        consumer_->close();
    }
}

void KafkaConsumer::SetErrorHandler(ErrorHandler handler) {
    error_handler_ = handler;
}

void KafkaConsumer::SetMessageHandler(MessageHandler handler) {
    message_handler_ = handler;
}

void KafkaConsumer::Run() {
    if (!consumer_) {
        return;
    }

    running_ = true;
    while (running_) {
        std::unique_ptr<RdKafka::Message> message{consumer_->consume(1000)};

        switch (message->err()) {
        case RdKafka::ERR_NO_ERROR: {
            std::string key = message->key() ? *message->key() : std::string{};
            std::string payload{static_cast<const char*>(message->payload()), message->len()};
            if (message_handler_) {
                message_handler_(key, payload);
            }
            // Committed only after the handler returned: a message that was not
            // stored has to be delivered again.
            consumer_->commitSync();
            break;
        }
        case RdKafka::ERR__TIMED_OUT:
        case RdKafka::ERR__PARTITION_EOF:
            break;
        default:
            ReportError(message->errstr());
            break;
        }
    }
}

void KafkaConsumer::Stop() {
    running_ = false;
}

void KafkaConsumer::ReportError(const std::string& message) {
    if (error_handler_) {
        error_handler_(message);
    }
}

}   //kafka
