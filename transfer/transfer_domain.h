#ifndef TRANSFER_DOMAIN_H
#define TRANSFER_DOMAIN_H

#include <QScopedPointer>
#include <QStringList>

class TransferInterface;

namespace transfer {

enum class TransferType {
    SERIAL,
    TCP
};

struct TransferSettings {
    virtual ~TransferSettings() = default;
};

} //transfer namespace

#endif // TRANSFER_DOMAIN_H
