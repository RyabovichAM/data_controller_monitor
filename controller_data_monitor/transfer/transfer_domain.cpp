#include "transfer_domain.h"

namespace transfer {

QScopedPointer<TransferInterface> CreateTransfer(TransferType type,
                QStringList&& settings) {
    switch(type) {
    case TransferType::SERIAL:
        // return QScopedPointer<SerialTransfer>(std::move(settings));

    case TransferType::TCP:
        // return QScopedPointer<TCPTransfer>(std::move(settings));

    default:
        throw("transfer::CreateTransfer: transfer type don't exist");
    }
}


}
