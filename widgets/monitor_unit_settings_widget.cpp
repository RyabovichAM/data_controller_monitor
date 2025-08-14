#include "monitor_unit_settings_widget.h"
#include "ui_monitor_unit_settings_widget.h"

MonitorUnitSettingsWidget::MonitorUnitSettingsWidget(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::MonitorUnitSettingsWidget)
{
    ui->setupUi(this);
    setModal(true);

    //view widget
    ui->tabWidget->setTabText(0, "View");
    view_widget_ = new ViewWidget(ui->view_tab);
    QVBoxLayout* view_layout = new QVBoxLayout(ui->view_tab);
    view_layout->addWidget(view_widget_);
    ui->view_tab->setLayout(view_layout);

    //transfer widget
    ui->tabWidget->setTabText(1, "Transfer");
    transfer_stg_wgt_ = new TransferSettingsWidget(ui->transfer_tab);
    QVBoxLayout* transfer_layout = new QVBoxLayout(ui->transfer_tab);
    transfer_layout->addWidget(transfer_stg_wgt_);
    ui->transfer_tab->setLayout(transfer_layout);
}

MonitorUnitSettingsWidget::~MonitorUnitSettingsWidget()
{
    delete ui;
}


QWidget* MonitorUnitSettingsWidget::GetWidget() const {
    return view_widget_->GetDropArea();
}

const app::MonitorUnitSettings MonitorUnitSettingsWidget::GetSettings() const {
    app::MonitorUnitSettings result;
    result.transfer = transfer_stg_wgt_->GetSettings();
    return result;
}
