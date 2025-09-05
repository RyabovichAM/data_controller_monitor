#ifndef TCPIP_TRANSFER_DOMAIN_H
#define TCPIP_TRANSFER_DOMAIN_H

#include <QHash>
#include <QHostAddress>

#include "transfer_domain.h"

namespace transfer {

QHostAddress GetHostFromString(const QString& host_str);

struct TcpIpSettings : public TransferSettings {
    QHostAddress host = QHostAddress::Any;
    quint16 port = 0;
};

TcpIpSettings GetTcpIpSettingsFromHashMap(const QHash<QString, QString>& settings_map);

}   //transfer

#endif // TCPIP_TRANSFER_DOMAIN_H
