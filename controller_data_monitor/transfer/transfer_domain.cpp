#include "transfer_domain.h"


QScopedPointer<TransferInterface> CreateTransfer(TransferType type) {
    switch(type) {
    case TransferType::SERIAL:
        return QScopedPointer<
    }
}
