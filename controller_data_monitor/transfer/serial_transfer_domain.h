#ifndef SERIAL_TRANSFER_DOMAIN_H
#define SERIAL_TRANSFER_DOMAIN_H

#include <QSerialPort>

namespace transfer {

QSerialPort::BaudRate getBaudRateFromString(QString baudRateStr);
QSerialPort::DataBits getDataBitsFromString(QString dataBitsStr);
QSerialPort::Parity getParityFromString(QString parityStr);
QSerialPort::StopBits getStopBitsFromString(QString stopBitsStr);
QSerialPort::FlowControl getFlowControlFromString(QString flowControlStr);

struct SerialSettings {
    QSerialPort::BaudRate       baudRate;
    QSerialPort::DataBits       dataBits;
    QSerialPort::Parity         parity;
    QSerialPort::StopBits       stopBits;
    QSerialPort::FlowControl    flowControl;
};

/*
    @IMPORTANT, the structure of the QStringList must be:
    list[0] is BaudRate;
    list[1] is DataBits;
    list[2] is Parity;
    list[3] is StopBits;
    list[4] is FlowControl;
*/
SerialSettings getSerialSettingsFromList(QStringList&& lst);

} // namespace transfer

#endif // SERIAL_TRANSFER_DOMAIN_H
