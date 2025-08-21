#ifndef TRANSFER_INTERFACE_H
#define TRANSFER_INTERFACE_H

#include <QObject>
#include <QJsonParseError>

#include "transfer_domain.h"

namespace transfer {

using OpenErrorHandler = std::function<void(QString)>;
using ReceivedDataHandler = std::function<void()>;

class TransferInterface : public QObject {
    Q_OBJECT
public:
    explicit TransferInterface(QObject *parent = nullptr) : QObject{parent} {
    }

    virtual ~TransferInterface() {
    }

    virtual void SetUp(TransferSettings* settings) = 0;
    virtual void Run(QIODeviceBase::OpenMode mode, OpenErrorHandler err_handler) = 0;
    virtual void Stop() = 0;
    virtual void SetReceivedDataHandler(ReceivedDataHandler handler) = 0;
    virtual bool ReadJsonLine() = 0;

};

}   //transfer namespace

#endif // TRANSFER_INTERFACE_H
