#ifndef TRANSFER_INTERFACE_H
#define TRANSFER_INTERFACE_H

#include <QObject>
namespace transfer {

class TransferInterface : public QObject {
    Q_OBJECT
public:
    explicit TransferInterface(QObject *parent = nullptr) {
    }
    virtual ~TransferInterface() {
    }

    // Открыть соединение
    virtual bool open() = 0;

    // Закрыть соединение
    virtual void close() = 0;

    // Отправить данные
    virtual qint64 write(const QByteArray &data) = 0;

    // Прочитать данные
    virtual QByteArray readAll() = 0;

    // Проверить, открыто ли соединение
    virtual bool isOpen() const = 0;

signals:
    // Сигнал о получении новых данных
    void dataReceived(const QByteArray &data);

    // Сигнал об ошибке
    void errorOccurred(const QString &errorString);

    // Сигнал об изменении состояния соединения
    void connectionStateChanged(bool isConnected);
};

}   //transfer namespace

#endif // TRANSFER_INTERFACE_H
