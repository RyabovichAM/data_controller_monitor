#ifndef SERIAL_TRANSFER_H
#define SERIAL_TRANSFER_H

#include <QJsonDocument>
#include <QSerialPort>
#include <QTextStream>
#include <QDebug>

#include "transfer_domain.h"
#include "transfer_interface.h"
#include "serial_transfer_domain.h"

namespace transfer {

class SerialTransfer : public TransferInterface {
    Q_OBJECT
public:
    explicit SerialTransfer(const QHash<QString,QString>& settings, QObject *parent = nullptr)
        :   settings_{GetSerialSettingsFromHashMap(settings)},
            TransferInterface{parent} {

    }

    SerialTransfer(QObject *parent = nullptr)
        : TransferInterface{parent} {

    }

    void SetUp(TransferSettings* settings) override {
        auto serial_set = dynamic_cast<SerialSettings*>(settings);

        if(!serial_set)
            Q_ASSERT("SerialTransfer::SetUp: wrong cast");

        serial_port_.setPortName(serial_set->port_name);
        serial_port_.setBaudRate(serial_set->baud_rate);
        serial_port_.setDataBits(serial_set->data_bits);
        serial_port_.setParity(serial_set->parity);
        serial_port_.setStopBits(serial_set->stop_bits);
        serial_port_.setFlowControl(serial_set->flow_control);
    }

    void Run(QIODeviceBase::OpenMode mode, OpenErrorHandler err_handler) override {
        if(serial_port_.open(mode)) {
        } else {
            err_handler(serial_port_.errorString());
        }
    }

    void Stop() override {
        serial_port_.close();
    }

    void SetJsonReceivedDataHandler(JsonReceivedDataHandler handler) override {
        json_received_data_handler_ = handler;
    }

    void SetErrorOcccuredHandler(ErrorOcccuredHandler handler) override {
        disconnect(&serial_port_,&QSerialPort::errorOccurred, nullptr, nullptr);
        connect(&serial_port_, &QSerialPort::errorOccurred, this, [&handler](QSerialPort::SerialPortError error) {
            if (error != QSerialPort::NoError) {
                handler();
            }
        });
    }

    bool ReadJsonLine() override {
        QTextStream stream{&serial_port_};
        QString line = stream.readLine().trimmed();
        QJsonParseError error;
        QJsonDocument data = QJsonDocument::fromJson(line.toUtf8(), &error);

        if(error.error == QJsonParseError::NoError) {
            json_received_data_handler_(data);
            return true;
        }
        return false;
    }

    // bool readDataLine(QVector<QByteArray>& data, char separator = ',') override {
    //     buffer_ += serial_port_.readAll();
    //     QVector<QByteArray> ring;
    //     ring.reserve(data.size());
    //     size_t end_of_the_ring = 0;

    //     bool first_separator = true;
    //     // bool first_end_line = true;
    //     QByteArray unit;

    //     for (auto byte : buffer_) {
    //         if(ring.size() == data.size()) {
    //             break;
    //         }

    //         if(byte == '\n') {
    //             end_of_the_ring = ring.size();
    //             //data before first separator not full (not valid)
    //             if(first_separator) {
    //                 first_separator = false;
    //                 unit.clear();
    //             } else {
    //                 ring.push_back(std::move(unit));
    //                 unit.clear();
    //             }
    //             continue;
    //         }

    //         if(byte == separator) {

    //             //data before first separator not full (not valid)
    //             if(first_separator) {
    //                 first_separator = false;
    //                 unit.clear();
    //             } else {
    //                 ring.push_back(std::move(unit));
    //                 unit.clear();
    //             }
    //             continue;
    //         }
    //         unit.push_back(byte);
    //     }

    //     // if()
    //     return false;


    // }

private:
    SerialSettings settings_;
    QSerialPort serial_port_;
    JsonReceivedDataHandler json_received_data_handler_;
};

}   //transfer namespace

#endif // SERIAL_TRANSFER_H
