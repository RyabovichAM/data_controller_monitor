#ifndef SERIAL_TRANSFER_H
#define SERIAL_TRANSFER_H

#include <QSerialPort>
#include "transfer_interface.h"

namespace transfer {

class SerialTransfer : public TransferInterface {
    Q_OBJECT
public:
    explicit SerialTransfer(const QString &portName, QObject *parent = nullptr)
        : Transfer(parent), m_portName(portName), m_serialPort(new QSerialPort(this)) {
        connect(m_serialPort, &QSerialPort::readyRead, this, [this]() {
            emit dataReceived(readAll());
        });
        connect(m_serialPort, &QSerialPort::errorOccurred, this, [this](QSerialPort::SerialPortError error) {
            if (error != QSerialPort::NoError) {
                emit errorOccurred(m_serialPort->errorString());
            }
        });
    }

    bool open() override {
        m_serialPort->setPortName(m_portName);
        // Установка параметров по умолчанию
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
        return m_serialPort->readAll();
    }

    bool isOpen() const override {
        return m_serialPort->isOpen();
    }

    // Дополнительные методы для настройки параметров порта
    void setBaudRate(qint32 baudRate) {
        m_serialPort->setBaudRate(baudRate);
    }

    void setDataBits(QSerialPort::DataBits dataBits) {
        m_serialPort->setDataBits(dataBits);
    }

    void setParity(QSerialPort::Parity parity) {
        m_serialPort->setParity(parity);
    }

    void setStopBits(QSerialPort::StopBits stopBits) {
        m_serialPort->setStopBits(stopBits);
    }

    void setFlowControl(QSerialPort::FlowControl flowControl) {
        m_serialPort->setFlowControl(flowControl);
    }

private:
    QString m_portName;
    QSerialPort *m_serialPort;
};

}   //transfer namespace

#endif // SERIAL_TRANSFER_H
