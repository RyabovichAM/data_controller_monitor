#include "monitor_unit_settings_widget.h"
#include "ui_monitor_unit_settings_widget.h"

MonitorUnitSettingsWidget::MonitorUnitSettingsWidget(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::MonitorUnitSettingsWidget)
{
    ui->setupUi(this);
    setModal(true);

    //transfer widget
    ui->tabWidget->setTabText(1, "Transfer");
    transfer_stg_wgt_ = new TransferSettingsWidget(ui->transfer_tab);
    QVBoxLayout *transfer_layout = new QVBoxLayout(ui->transfer_tab);
    transfer_layout->addWidget(transfer_stg_wgt_);
    ui->transfer_tab->setLayout(transfer_layout);
}

MonitorUnitSettingsWidget::~MonitorUnitSettingsWidget()
{
    delete ui;
}
