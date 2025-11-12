#include "transfer_settings_widget.h"
#include "ui_transfer_settings_widget.h"

#include <QSerialPortInfo>
#include <QMessageBox>

TransferSettingsWidget::TransferSettingsWidget(QWidget *parent)
    : QTabWidget(parent)
    , ui(new Ui::TransferSettingsWidget)
{
    ui->setupUi(this);

    for (const QSerialPortInfo &port : QSerialPortInfo::availablePorts()) {
        ui->port_name->addItem(port.portName());
    }
}

TransferSettingsWidget::~TransferSettingsWidget() {
    delete ui;
}


std::optional<QHash<QString, QString>> TransferSettingsWidget::GetSettings()
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
        if (lineedit->text().trimmed().isEmpty()) {
            QMessageBox::warning(this, "Error", "Field cannot be empty");
            lineedit->setFocus();
            return std::nullopt;
        }
        settings[lineedit->objectName()] = lineedit->text();
    }

    return settings;
}
