#ifndef TRANSFER_DOMAIN_H
#define TRANSFER_DOMAIN_H

#include <QScopedPointer>

class TransferInterface;

namespace transfer {

enum class TransferType {
    SERIAL,
    TCP
};

QScopedPointer<TransferInterface> CreateTransfer(TransferType type);

} //transfer namespace

#endif // TRANSFER_DOMAIN_H
