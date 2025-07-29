#include "serial_transfer_domain.h"



namespace transfer {


QSerialPort::BaudRate getBaudRateFromString(QString baudRateStr) {
    if (baudRateStr == "1200") return QSerialPort::Baud1200;
    else if (baudRateStr == "2400") return QSerialPort::Baud2400;
    else if (baudRateStr == "4800") return QSerialPort::Baud4800;
    else if (baudRateStr == "9600") return QSerialPort::Baud9600;
    else if (baudRateStr == "19200") return QSerialPort::Baud19200;
    else if (baudRateStr == "38400") return QSerialPort::Baud38400;
    else if (baudRateStr == "57600") return QSerialPort::Baud57600;
    else if (baudRateStr == "115200") return QSerialPort::Baud115200;
    else throw std::invalid_argument("transfer::getBaudRateFromString: invalid argument");
}

QSerialPort::DataBits getDataBitsFromString(QString dataBitsStr) {
    if (dataBitsStr == "5") return QSerialPort::Data5;
    else if (dataBitsStr == "6") return QSerialPort::Data6;
    else if (dataBitsStr == "7") return QSerialPort::Data7;
    else if (dataBitsStr == "8") return QSerialPort::Data8;
    else throw std::invalid_argument("transfer::getDataBitsFromString: invalid argument");
}

QSerialPort::Parity getParityFromString(QString parityStr) {
    if (parityStr == "None") return QSerialPort::NoParity;
    else if (parityStr == "Even") return QSerialPort::EvenParity;
    else if (parityStr == "Odd") return QSerialPort::OddParity;
    else if (parityStr == "Space") return QSerialPort::SpaceParity;
    else if (parityStr == "Mark") return QSerialPort::MarkParity;
    else throw std::invalid_argument("transfer::getParityFromString: invalid argument");
}

QSerialPort::StopBits getStopBitsFromString(QString stopBitsStr) {
    if (stopBitsStr == "1") return QSerialPort::OneStop;
    else if (stopBitsStr == "1.5") return QSerialPort::OneAndHalfStop;
    else if (stopBitsStr == "2") return QSerialPort::TwoStop;
    else throw std::invalid_argument("transfer::getStopBitsFromString: invalid argument");
}

QSerialPort::FlowControl getFlowControlFromString(QString flowControlStr) {
    if (flowControlStr == "None") return QSerialPort::NoFlowControl;
    else if (flowControlStr == "Hardware") return QSerialPort::HardwareControl;
    else if (flowControlStr == "Software") return QSerialPort::SoftwareControl;
    else throw std::invalid_argument("transfer::getStopBitsFromString: invalid argument");
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
SerialSettings getSerialSettingsFromList(QStringList&& lst) {
    SerialSettings settings;
    settings.port_name = std::move(lst[0]);
    settings.baud_rate = getBaudRateFromString(std::move(lst[1]));
    settings.data_bits = getDataBitsFromString(std::move(lst[2]));
    settings.parity = getParityFromString(std::move(lst[3]));
    settings.stop_bits = getStopBitsFromString(std::move(lst[4]));
    settings.flow_control = getFlowControlFromString(std::move(lst[5]));

    return settings;
}

}
