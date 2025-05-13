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
