#ifndef SERIAL_TRANSFER_H
#define SERIAL_TRANSFER_H

#include <QJsonDocument>
#include <QTextStream>

#include "transfer_domain.h"
#include "transfer_interface.h"
#include "serial_transfer_domain.h"

namespace transfer {

class SerialTransfer : public TransferInterface {
    Q_OBJECT
public:
    explicit SerialTransfer(const QHash<QString,QString>& settings,
                            QObject *parent = nullptr);
    SerialTransfer(QObject *parent = nullptr);

    void SetUp(TransferSettings* settings) override;
    void Run(QIODeviceBase::OpenMode mode, OpenErrorHandler err_handler) override;
    void Stop() override;
    void SetJsonReceivedDataHandler(JsonReceivedDataHandler handler) override;
    void SetErrorOcccuredHandler(ErrorOcccuredHandler handler) override;
    bool ReadJsonLine() override;

private:
    SerialSettings settings_;
    QSerialPort serial_port_;
    JsonReceivedDataHandler json_received_data_handler_;

    QByteArray data_buffer_;
    qsizetype left_idx{-1};
    qsizetype right_idx{-1};

    void SetUp(const SerialSettings& settings) {
        serial_port_.setPortName(settings.port_name);
        serial_port_.setBaudRate(settings.baud_rate);
        serial_port_.setDataBits(settings.data_bits);
        serial_port_.setParity(settings.parity);
        serial_port_.setStopBits(settings.stop_bits);
        serial_port_.setFlowControl(settings.flow_control);
    }
};

}   //transfer namespace

#endif // SERIAL_TRANSFER_H
