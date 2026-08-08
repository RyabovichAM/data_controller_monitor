#include "kafka_consumer.h"

namespace kafka {

KafkaConsumer::KafkaConsumer(const QString& brokers, const QString& topic,
                             const QString& group_id) {
    std::string errstr;
    std::unique_ptr<RdKafka::Conf> conf{
        RdKafka::Conf::create(RdKafka::Conf::CONF_GLOBAL)};

    conf->set("bootstrap.servers", brokers.toStdString(), errstr);
    conf->set("group.id", group_id.toStdString(), errstr);
    conf->set("auto.offset.reset", "earliest", errstr);
    conf->set("enable.auto.commit", "false", errstr);

    consumer_.reset(RdKafka::KafkaConsumer::create(conf.get(), errstr));
    if (!consumer_) {
        ReportError(QString::fromStdString(errstr));
        return;
    }

    RdKafka::ErrorCode err = consumer_->subscribe({topic.toStdString()});
    if (err != RdKafka::ERR_NO_ERROR) {
        ReportError(QString::fromStdString(RdKafka::err2str(err)));
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
            QString key = message->key() ? QString::fromStdString(*message->key())
                                         : QString{};
            QByteArray payload{static_cast<const char*>(message->payload()),
                               static_cast<int>(message->len())};
            if (message_handler_) {
                message_handler_(key, payload);
            }
            consumer_->commitSync();
            break;
        }
        case RdKafka::ERR__TIMED_OUT:
        case RdKafka::ERR__PARTITION_EOF:
            break;
        default:
            ReportError(QString::fromStdString(message->errstr()));
            break;
        }
    }
}

void KafkaConsumer::Stop() {
    running_ = false;
}

void KafkaConsumer::ReportError(const QString& message) {
    if (error_handler_) {
        error_handler_(message);
    }
}

}   //kafka
