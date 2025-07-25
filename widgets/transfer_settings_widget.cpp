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


void TransferSettingsWidget::GetSettings()
{
    // int index = this->currentIndex();
    // switch(index) {
    //     case 0:
    //     return std::make_unique<SerialTransfer>()
    //         break;
    //     case 1:

    // }
}
