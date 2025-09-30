#include "monitor_unit_settings_widget.h"
#include "ui_monitor_unit_settings_widget.h"

MonitorUnitSettingsWidget::MonitorUnitSettingsWidget(view_widget::Canvas* parent)
    : QDialog(parent)
    , ui(new Ui::MonitorUnitSettingsWidget)
{
    ui->setupUi(this);
    setModal(true);

    //view widget
    view_widget_ = new view_widget::ViewWidget(ui->view_tab);
    QVBoxLayout* view_layout = new QVBoxLayout(ui->view_tab);
    view_layout->addWidget(view_widget_);
    ui->view_tab->setLayout(view_layout);

    //transfer widget
    transfer_stg_wgt_ = new TransferSettingsWidget(ui->transfer_tab);
    QVBoxLayout* transfer_layout = new QVBoxLayout(ui->transfer_tab);
    transfer_layout->addWidget(transfer_stg_wgt_);
    transfer_layout->addStretch(1);
    ui->transfer_tab->setLayout(transfer_layout);

    //data storage widget
    storage_stg_wgt = new DataStorageSettingsWidget(ui->storage_tab);
    QVBoxLayout* storage_layout = new QVBoxLayout(ui->storage_tab);
    storage_layout->addWidget(storage_stg_wgt);
    ui->storage_tab->setLayout(storage_layout);

}

MonitorUnitSettingsWidget::~MonitorUnitSettingsWidget() {
    delete ui;
}

view_widget::Canvas* MonitorUnitSettingsWidget::GetWidget() const {
    return view_widget_->GetCanvas();
}

const app::MonitorUnitSettings MonitorUnitSettingsWidget::GetSettings() const {
    app::MonitorUnitSettings result;
    result.transfer = transfer_stg_wgt_->GetSettings();
    result.data_storage = storage_stg_wgt->GetSettings();
    return result;
}
