#ifndef TRANSFER_SETTINGS_WIDGET_H
#define TRANSFER_SETTINGS_WIDGET_H

#include <QTabWidget>

namespace Ui {
class TransferSettingsWidget;
}

class TransferSettingsWidget : public QTabWidget
{
    Q_OBJECT

public:
    explicit TransferSettingsWidget(QWidget *parent = nullptr);
    ~TransferSettingsWidget();

    QHash<QString, QString> GetSettings();

private:
    Ui::TransferSettingsWidget *ui;
};

#endif // TRANSFER_SETTINGS_WIDGET_H
