#include "monitor_unit_settings_widget.h"
#include "ui_monitor_unit_settings_widget.h"

#include <QApplication>
#include <QPushButton>

MonitorUnitSettingsWidget::MonitorUnitSettingsWidget(view_widget::Canvas* parent)
    : QDialog(parent)
    , ui(new Ui::MonitorUnitSettingsWidget)
{
    ui->setupUi(this);
    setWindowTitle("Monitor Unit Settings");
    setModal(true);
    disconnect(ui->buttonBox, &QDialogButtonBox::accepted, 0, 0);
    disconnect(ui->buttonBox->button(QDialogButtonBox::Ok), &QPushButton::clicked, 0, 0);
    connect(ui->buttonBox->button(QDialogButtonBox::Ok), &QPushButton::clicked,
            this, &MonitorUnitSettingsWidget::ValidateAndAccept);

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

    QScreen *screen = QApplication::primaryScreen();
    QRect screenGeometry = screen->geometry();
    resize(screenGeometry.width() * 0.3, screenGeometry.height() * 0.4);
}

MonitorUnitSettingsWidget::~MonitorUnitSettingsWidget() {
    delete ui;
}

view_widget::Canvas* MonitorUnitSettingsWidget::GetWidget() const {
    return view_widget_->GetCanvas();
}

void MonitorUnitSettingsWidget::ValidateAndAccept() {
    if(transfer_stg_wgt_->GetSettings()) {
        mu_settings_.transfer = *(transfer_stg_wgt_->GetSettings());
    } else {
        ui->tabWidget->setCurrentWidget(ui->transfer_tab);
        return;
    }
    if(storage_stg_wgt->GetSettings()) {
        mu_settings_.data_storage = *(storage_stg_wgt->GetSettings());
    } else {
        ui->tabWidget->setCurrentWidget(ui->storage_tab);
        return;
    }
    accept();
}

const app::MonitorUnitSettings MonitorUnitSettingsWidget::GetSettings() const {
    return mu_settings_;
}
