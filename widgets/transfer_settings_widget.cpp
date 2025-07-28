#include "transfer_settings_widget.h"
#include "ui_transfer_settings_widget.h"

TransferSettingsWidget::TransferSettingsWidget(QWidget *parent)
    : QTabWidget(parent)
    , ui(new Ui::TransferSettingsWidget)
{
    ui->setupUi(this);
}

TransferSettingsWidget::~TransferSettingsWidget()
{
    delete ui;
}


QHash<QString, QString> TransferSettingsWidget::GetSettings()
{
    QHash<QString, QString> settings;
    auto widget = currentWidget();

    QList<QComboBox*> comboboxes = widget->findChildren<QComboBox*>();

    for (QComboBox* combo : comboboxes) {
        settings[combo->objectName()] = combo->currentText();
    }
    return settings;
}
