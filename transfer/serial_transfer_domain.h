#ifndef SERIAL_TRANSFER_DOMAIN_H
#define SERIAL_TRANSFER_DOMAIN_H

#include <QHash>
#include <QSerialPort>

#include "transfer_domain.h"

namespace transfer {

QSerialPort::BaudRate GetBaudRateFromString(QString baudRateStr);
QSerialPort::DataBits GetDataBitsFromString(QString dataBitsStr);
QSerialPort::Parity GetParityFromString(QString parityStr);
QSerialPort::StopBits GetStopBitsFromString(QString stopBitsStr);
QSerialPort::FlowControl GetFlowControlFromString(QString flowControlStr);

struct SerialSettings : public TransferSettings {
    QString                     port_name;
    QSerialPort::BaudRate       baud_rate = QSerialPort::Baud9600;
    QSerialPort::DataBits       data_bits = QSerialPort::Data8;
    QSerialPort::Parity         parity = QSerialPort::NoParity;
    QSerialPort::StopBits       stop_bits = QSerialPort::OneStop;
    QSerialPort::FlowControl    flow_control = QSerialPort::NoFlowControl;
    QIODeviceBase::OpenMode     open_mode = QIODeviceBase::ReadOnly;
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
SerialSettings GetSerialSettingsFromList(QStringList&& lst);

SerialSettings GetSerialSettingsFromHashMap(const QHash<QString, QString>& settings_map);

} // namespace transfer

#endif // SERIAL_TRANSFER_DOMAIN_H
