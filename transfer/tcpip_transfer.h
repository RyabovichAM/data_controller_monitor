#ifndef TCPIP_TRANSFER_H
#define TCPIP_TRANSFER_H

#include <QTcpServer>

#include "transfer_interface.h"
#include "tcpip_transfer_domain.h"

namespace transfer {

class TcpIpTransfer : public TransferInterface
{
    Q_OBJECT
public:
    explicit TcpIpTransfer(const QHash<QString,QString>& settings,
                  QObject *parent = nullptr);
    explicit TcpIpTransfer(QObject *parent = nullptr);

    void SetUp(TransferSettings* settings) override;
    void Run(OpenErrorHandler err_handler) override;
    void Stop() override;
    void SetJsonReceivedDataHandler(JsonReceivedDataHandler handler) override;
    void SetErrorOcccuredHandler(ErrorOcccuredHandler handler) override;
    bool ReadJsonLine() override;

public slots:
    void OnNewConnection();

private:
    QTcpServer tcp_server_;
    TcpIpSettings settings_;
    JsonReceivedDataHandler json_received_data_handler_;
    ErrorOcccuredHandler  error_occcured_handler_;

    QByteArray data_buffer_;
    qsizetype left_idx_{-1};
    qsizetype right_idx_{-1};
};

}   //transfer

#endif // TCPIP_TRANSFER_H
