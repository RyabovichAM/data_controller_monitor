#ifndef APP_DOMAIN_H
#define APP_DOMAIN_H

#include <QHash>
#include <QString>

namespace app {

struct MonitorUnitSettings {
    QHash<QString, QString> transfer;
};


}   //app


#endif // APP_DOMAIN_H
