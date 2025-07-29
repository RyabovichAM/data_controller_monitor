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
    QString                     port_name;
    QSerialPort::BaudRate       baud_rate = QSerialPort::Baud9600;
    QSerialPort::DataBits       data_bits = QSerialPort::Data8;
    QSerialPort::Parity         parity = QSerialPort::NoParity;
    QSerialPort::StopBits       stop_bits = QSerialPort::OneStop;
    QSerialPort::FlowControl    flow_control = QSerialPort::NoFlowControl;
    QIODeviceBase::OpenMode     open_mode;
};

/*
    @IMPORTANT, the structure of the QStringList must be:
    list[0] is PortName;
    list[1] is BaudRate;
    list[2] is DataBits;
    list[3] is Parity;
    list[4] is StopBits;
    list[5] is FlowControl;
*/
SerialSettings getSerialSettingsFromList(QStringList&& lst);

} // namespace transfer

#endif // SERIAL_TRANSFER_DOMAIN_H
