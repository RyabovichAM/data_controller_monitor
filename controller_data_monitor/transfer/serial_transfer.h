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

        serial_port_->setBaudRate(settings_.baudRate);
        serial_port_->setDataBits(settings_.dataBits);
        serial_port_->setParity(settings_.parity);
        serial_port_->setStopBits(settings_.stopBits);
        serial_port_->setFlowControl(settings_.flowControl);

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
        m_serialPort->setBaudRate(QSerialPort::Baud9600);
        m_serialPort->setDataBits(QSerialPort::Data8);
        m_serialPort->setParity(QSerialPort::NoParity);
        m_serialPort->setStopBits(QSerialPort::OneStop);
        m_serialPort->setFlowControl(QSerialPort::NoFlowControl);

        if (m_serialPort->open(QIODevice::ReadWrite)) {
            emit connectionStateChanged(true);
            return true;
        }
        return false;
    }

    void close() override {
        m_serialPort->close();
        emit connectionStateChanged(false);
    }

    qint64 write(const QByteArray &data) override {
        return m_serialPort->write(data);
    }

    QByteArray readAll() override {
        return serial_port_.readAll();
    }

    bool isOpen() const override {
        return serial_port_.isOpen();
    }

    // Дополнительные методы для настройки параметров порта
    void setBaudRate(qint32 baudRate) {
        serial_port_.setBaudRate(baudRate);
    }

    void setDataBits(QSerialPort::DataBits dataBits) {
        serial_port_.>setDataBits(dataBits);
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
};

}   //transfer namespace

#endif // SERIAL_TRANSFER_H
