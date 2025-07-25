#ifndef SERIAL_TRANSFER_H
#define SERIAL_TRANSFER_H

#include <QSerialPort>
#include "transfer_interface.h"
#include "serial_transfer_domain.h"

namespace transfer {

class SerialTransfer : public TransferInterface {
    Q_OBJECT
public:
    explicit SerialTransfer(SerialSettings&& settings, QObject *parent = nullptr)
        :   settings_{std::move(settings)},
            TransferInterface{parent},
            serial_port_{settings_.port_name} {

        serial_port_.setBaudRate(settings_.baudRate);
        serial_port_.setDataBits(settings_.dataBits);
        serial_port_.setParity(settings_.parity);
        serial_port_.setStopBits(settings_.stopBits);
        serial_port_.setFlowControl(settings_.flowControl);

        // connect(serial_port_, &QSerialPort::readyRead, this, [this]() {
        //     emit dataReceived(readAll());
        // });
        // connect(serial_port_, &QSerialPort::errorOccurred, this, [this](QSerialPort::SerialPortError error) {
        //     if (error != QSerialPort::NoError) {
        //         emit errorOccurred(serial_port_->errorString());
        //     }
        // });
    }

    bool open() override {
        return serial_port_.open(settings_.open_mode);
    }

    void close() override {
        serial_port_.close();
    }

    qint64 write(const QByteArray& data) override {
        return serial_port_.write(data);
    }

    QByteArray readAll() override {
        return serial_port_.readAll();
    }

    bool readDataLine(QVector<QByteArray>& data, char separator = ',') override {
        buffer_ += serial_port_.readAll();
        QVector<QByteArray> ring;
        ring.reserve(data.size());
        size_t end_of_the_ring = 0;

        bool first_separator = true;
        // bool first_end_line = true;
        QByteArray unit;

        for (auto byte : buffer_) {
            if(ring.size() == data.size()) {
                break;
            }

            if(byte == '\n') {
                end_of_the_ring = ring.size();
                //data before first separator not full (not valid)
                if(first_separator) {
                    first_separator = false;
                    unit.clear();
                } else {
                    ring.push_back(std::move(unit));
                    unit.clear();
                }
                continue;
            }

            if(byte == separator) {

                //data before first separator not full (not valid)
                if(first_separator) {
                    first_separator = false;
                    unit.clear();
                } else {
                    ring.push_back(std::move(unit));
                    unit.clear();
                }
                continue;
            }
            unit.push_back(byte);
        }

        // if()
        return false;


    }

    bool isOpen() const override {
        return serial_port_.isOpen();
    }

    void setBaudRate(qint32 baudRate) {
        serial_port_.setBaudRate(baudRate);
    }

    void setDataBits(QSerialPort::DataBits dataBits) {
        serial_port_.setDataBits(dataBits);
    }

    void setParity(QSerialPort::Parity parity) {
        serial_port_.setParity(parity);
    }

    void setStopBits(QSerialPort::StopBits stopBits) {
        serial_port_.setStopBits(stopBits);
    }

    void setFlowControl(QSerialPort::FlowControl flowControl) {
        serial_port_.setFlowControl(flowControl);
    }

private:
    SerialSettings settings_;
    QSerialPort serial_port_;
    QByteArray buffer_;
};

}   //transfer namespace

#endif // SERIAL_TRANSFER_H
