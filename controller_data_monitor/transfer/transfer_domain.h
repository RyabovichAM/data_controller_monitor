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
    TransferType type;
    QStringList settings;
};



QScopedPointer<TransferInterface> CreateTransfer(TransferType type,
                        QStringList&& settings);




} //transfer namespace

#endif // TRANSFER_DOMAIN_H
