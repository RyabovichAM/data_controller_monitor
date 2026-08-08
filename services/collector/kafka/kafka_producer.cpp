#include "kafka_producer.h"

namespace kafka {

KafkaProducer::KafkaProducer(const QString& brokers, const QString& topic)
    : topic_{topic} {
    std::string errstr;
    std::unique_ptr<RdKafka::Conf> conf{
        RdKafka::Conf::create(RdKafka::Conf::CONF_GLOBAL)};

    conf->set("bootstrap.servers", brokers.toStdString(), errstr);
    conf->set("enable.idempotence", "true", errstr);

    producer_.reset(RdKafka::Producer::create(conf.get(), errstr));
    if (!producer_) {
        ReportError(QString::fromStdString(errstr));
    }
}

KafkaProducer::~KafkaProducer() {
    if (producer_) {
        Flush();
    }
}

void KafkaProducer::SetErrorHandler(ErrorHandler handler) {
    error_handler_ = handler;
}

bool KafkaProducer::Publish(const QString& key, const QByteArray& payload) {
    if (!producer_) {
        return false;
    }

    std::string key_str = key.toStdString();
    RdKafka::ErrorCode err = producer_->produce(
        topic_.toStdString(),
        RdKafka::Topic::PARTITION_UA,
        RdKafka::Producer::RK_MSG_COPY,
        const_cast<char*>(payload.constData()), payload.size(),
        key_str.data(), key_str.size(),
        0, nullptr);

    if (err != RdKafka::ERR_NO_ERROR) {
        ReportError(QString::fromStdString(RdKafka::err2str(err)));
        return false;
    }

    producer_->poll(0);
    return true;
}

void KafkaProducer::Flush(int timeout_ms) {
    producer_->flush(timeout_ms);
}

void KafkaProducer::ReportError(const QString& message) {
    if (error_handler_) {
        error_handler_(message);
    }
}

}   //kafka
