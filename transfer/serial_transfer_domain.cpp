#include "serial_transfer_domain.h"

namespace transfer {

QSerialPort::BaudRate GetBaudRateFromString(QString baudRateStr) {
    if (baudRateStr == "1200") return QSerialPort::Baud1200;
    else if (baudRateStr == "2400") return QSerialPort::Baud2400;
    else if (baudRateStr == "4800") return QSerialPort::Baud4800;
    else if (baudRateStr == "9600") return QSerialPort::Baud9600;
    else if (baudRateStr == "19200") return QSerialPort::Baud19200;
    else if (baudRateStr == "38400") return QSerialPort::Baud38400;
    else if (baudRateStr == "57600") return QSerialPort::Baud57600;
    else if (baudRateStr == "115200") return QSerialPort::Baud115200;
    else throw std::invalid_argument("transfer::GetBaudRateFromString: invalid argument");
}

QSerialPort::DataBits GetDataBitsFromString(QString dataBitsStr) {
    if (dataBitsStr == "5") return QSerialPort::Data5;
    else if (dataBitsStr == "6") return QSerialPort::Data6;
    else if (dataBitsStr == "7") return QSerialPort::Data7;
    else if (dataBitsStr == "8") return QSerialPort::Data8;
    else throw std::invalid_argument("transfer::GetDataBitsFromString: invalid argument");
}

QSerialPort::Parity GetParityFromString(QString parityStr) {
    if (parityStr == "None") return QSerialPort::NoParity;
    else if (parityStr == "Even") return QSerialPort::EvenParity;
    else if (parityStr == "Odd") return QSerialPort::OddParity;
    else if (parityStr == "Space") return QSerialPort::SpaceParity;
    else if (parityStr == "Mark") return QSerialPort::MarkParity;
    else throw std::invalid_argument("transfer::GetParityFromString: invalid argument");
}

QSerialPort::StopBits GetStopBitsFromString(QString stopBitsStr) {
    if (stopBitsStr == "1") return QSerialPort::OneStop;
    else if (stopBitsStr == "1.5") return QSerialPort::OneAndHalfStop;
    else if (stopBitsStr == "2") return QSerialPort::TwoStop;
    else throw std::invalid_argument("transfer::GetStopBitsFromString: invalid argument");
}

QSerialPort::FlowControl GetFlowControlFromString(QString flowControlStr) {
    if (flowControlStr == "None") return QSerialPort::NoFlowControl;
    else if (flowControlStr == "Hardware") return QSerialPort::HardwareControl;
    else if (flowControlStr == "Software") return QSerialPort::SoftwareControl;
    else throw std::invalid_argument("transfer::GetStopBitsFromString: invalid argument");
}

/*
    @IMPORTANT, the structure of the QStringList must be:
    list[0] is PortName;
    list[1] is BaudRate;
    list[2] is DataBits;
    list[3] is Parity;
    list[4] is StopBits;
    list[5] is FlowControl;
*/
SerialSettings GetSerialSettingsFromList(QStringList&& lst) {
    SerialSettings settings;
    settings.port_name = std::move(lst[0]);
    settings.baud_rate = GetBaudRateFromString(std::move(lst[1]));
    settings.data_bits = GetDataBitsFromString(std::move(lst[2]));
    settings.parity = GetParityFromString(std::move(lst[3]));
    settings.stop_bits = GetStopBitsFromString(std::move(lst[4]));
    settings.flow_control = GetFlowControlFromString(std::move(lst[5]));

    return settings;
}

SerialSettings GetSerialSettingsFromHashMap(const QHash<QString, QString>& settings_map) {
    SerialSettings ser_settings;
    ser_settings.port_name = settings_map.value("port_name");
    ser_settings.baud_rate = GetBaudRateFromString(settings_map.value("baud_rate"));
    ser_settings.data_bits = GetDataBitsFromString(settings_map.value("data_bits"));
    ser_settings.parity = GetParityFromString(settings_map.value("parity"));
    ser_settings.stop_bits = GetStopBitsFromString(settings_map.value("stop_bits"));
    ser_settings.flow_control = GetFlowControlFromString(settings_map.value("stop_bits"));
    return ser_settings;
}

}
