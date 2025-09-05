#include "tcpip_transfer.h"

#include <QJsonDocument>
#include <QTcpSocket>

namespace  transfer {

TcpIpTransfer::TcpIpTransfer(const QHash<QString,QString>& settings,
                                        QObject *parent)
    :   settings_{GetTcpIpSettingsFromHashMap(settings)}
    ,   TransferInterface{parent} {
}

TcpIpTransfer::TcpIpTransfer(QObject *parent)
    : TransferInterface{parent} {
}

void TcpIpTransfer::SetUp(TransferSettings* settings) {
    auto tcpip_settings = dynamic_cast<TcpIpSettings*>(settings);

    if(!tcpip_settings)
        Q_ASSERT("SerialTransfer::SetUp: wrong cast");

    settings_.host = tcpip_settings->host;
    settings_.port = tcpip_settings->port;
}

void TcpIpTransfer::Run(OpenErrorHandler err_handler) {
    connect(&tcp_server_, &QTcpServer::newConnection, this, &TcpIpTransfer::OnNewConnection);
    if(!tcp_server_.listen(settings_.host,settings_.port)) {
        tcp_server_.close();
        err_handler(tcp_server_.errorString());
    }
}

void TcpIpTransfer::Stop() {
    disconnect(&tcp_server_, &QTcpServer::newConnection, this, &TcpIpTransfer::OnNewConnection);
    tcp_server_.close();
}

void TcpIpTransfer::SetJsonReceivedDataHandler(JsonReceivedDataHandler handler) {
    json_received_data_handler_ = handler;
}

void TcpIpTransfer::SetErrorOcccuredHandler(ErrorOcccuredHandler handler) {
    error_occcured_handler_ = handler;
}

bool TcpIpTransfer::ReadJsonLine() {
    QTcpSocket* socket = static_cast<QTcpSocket*>(sender());

    data_buffer_ += socket->readAll();

    if(left_idx_ == -1)
        left_idx_ = data_buffer_.indexOf("{");
    if(right_idx_ == -1)
        right_idx_ = data_buffer_.indexOf("}");

    if(left_idx_ == -1 || right_idx_ == -1)
        return false;

    if(left_idx_ > right_idx_) {
        right_idx_ = -1;
        data_buffer_ = data_buffer_.mid(left_idx_);
        left_idx_ = 0;
        return false;
    }

    QJsonParseError error;
    QJsonDocument json_data = QJsonDocument::fromJson(data_buffer_.left(right_idx_+1), &error);

    data_buffer_ = data_buffer_.mid(right_idx_+1);
    left_idx_ = -1;
    right_idx_ = -1;

    if(error.error == QJsonParseError::NoError) {
        Q_ASSERT(json_received_data_handler_);
        json_received_data_handler_(json_data);
        return true;
    }

    return false;

}

void TcpIpTransfer::OnNewConnection() {
    QTcpSocket* socket = tcp_server_.nextPendingConnection();
    connect(socket, &QTcpSocket::disconnected, socket, &QTcpSocket::deleteLater);
    connect(socket, &QTcpSocket::readyRead, this, &TcpIpTransfer::ReadJsonLine);
    connect(socket, &QTcpSocket::errorOccurred, this , [self = this] () {
        if(self->error_occcured_handler_)
            self->error_occcured_handler_();
    });
}

}   //transfer

