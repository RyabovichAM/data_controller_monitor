#include "tcpip_transfer_domain.h"

namespace transfer {

QHostAddress GetHostFromString(const QString& host_str) {
    QHostAddress result = QHostAddress(host_str);
    if (result.isNull())
        throw std::invalid_argument("transfer::GetHostFromString:: invalid host argument");

    return result;
}

TcpIpSettings GetTcpIpSettingsFromHashMap(const QHash<QString, QString>& settings_map) {
    TcpIpSettings settings;
    settings.host = GetHostFromString(settings_map.value("host"));
    bool ok;
    quint16 port = settings_map.value("port").toUShort(&ok);
    if(!ok) {
        throw std::invalid_argument("transfer::GetTcpIpSettingsFromHashMap:: invalid port argument");
    }
    settings.port = port;
    return settings;
}


}   //transfer
