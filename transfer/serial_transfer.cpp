#include "serial_transfer.h"

namespace transfer {

SerialTransfer::SerialTransfer(const QHash<QString,QString>& settings,
                                        QObject *parent)
    :   settings_{GetSerialSettingsFromHashMap(settings)},
        TransferInterface{parent} {
    SetUp(settings_);
}

SerialTransfer::SerialTransfer(QObject *parent)
    : TransferInterface{parent} {
}

void SerialTransfer::SetUp(TransferSettings* settings)  {
    auto serial_set = dynamic_cast<SerialSettings*>(settings);

    if(!serial_set)
        Q_ASSERT("SerialTransfer::SetUp: wrong cast");

    SetUp(*serial_set);
}

void SerialTransfer::Run(QIODeviceBase::OpenMode mode, OpenErrorHandler err_handler) {
    if(serial_port_.open(mode)) {
        connect(&serial_port_, &QSerialPort::readyRead,this,&SerialTransfer::ReadJsonLine);
    } else {
        err_handler(serial_port_.errorString());
    }
}

void SerialTransfer::Stop() {
    disconnect(&serial_port_, &QSerialPort::readyRead,nullptr,nullptr);
    serial_port_.close();
}

void SerialTransfer::SetJsonReceivedDataHandler(JsonReceivedDataHandler handler) {
    json_received_data_handler_ = handler;
}

void SerialTransfer::SetErrorOcccuredHandler(ErrorOcccuredHandler handler) {
    disconnect(&serial_port_,&QSerialPort::errorOccurred, nullptr, nullptr);
    connect(&serial_port_, &QSerialPort::errorOccurred, this, [&handler](QSerialPort::SerialPortError error) {
        if (error != QSerialPort::NoError) {
            handler();
        }
    });
}

bool SerialTransfer::ReadJsonLine() {
    data_buffer_ += serial_port_.readAll();

    if(left_idx == -1)
        left_idx = data_buffer_.indexOf("{");
    if(right_idx == -1)
        right_idx = data_buffer_.indexOf("}");

    if(left_idx == -1 || right_idx == -1)
        return false;

    if(left_idx > right_idx) {
        right_idx = -1;
        data_buffer_ = data_buffer_.mid(left_idx);
        left_idx = 0;
        return false;
    }

    QJsonParseError error;
    QJsonDocument json_data = QJsonDocument::fromJson(data_buffer_.left(right_idx+1), &error);

    data_buffer_ = data_buffer_.mid(right_idx+1);
    left_idx == -1;
    right_idx = -1;

    if(error.error == QJsonParseError::NoError) {
        Q_ASSERT(json_received_data_handler_);
        json_received_data_handler_(json_data);
        return true;
    }

    return false;
}

}   //transfer namespace

