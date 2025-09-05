#include "transfer_settings_widget.h"
#include "ui_transfer_settings_widget.h"

#include <QSerialPortInfo>

TransferSettingsWidget::TransferSettingsWidget(QWidget *parent)
    : QTabWidget(parent)
    , ui(new Ui::TransferSettingsWidget)
{
    ui->setupUi(this);

    for (const QSerialPortInfo &port : QSerialPortInfo::availablePorts()) {
        ui->port_name->addItem(port.portName());
    }
}

TransferSettingsWidget::~TransferSettingsWidget()
{
    delete ui;
}


QHash<QString, QString> TransferSettingsWidget::GetSettings()
{
    QHash<QString, QString> settings;
    auto widget = currentWidget();

    settings["type"] = tabText(currentIndex());

    QList<QComboBox*> comboboxes = widget->findChildren<QComboBox*>();

    for (QComboBox* combo : comboboxes) {
        settings[combo->objectName()] = combo->currentText();
    }

    QList<QLineEdit*> lineedits = widget->findChildren<QLineEdit*>();

    for (QLineEdit* lineedit : lineedits) {
        settings[lineedit->objectName()] = lineedit->text();
    }

    return settings;
}
