#include "monitor_unit_settings_widget.h"
#include "ui_monitor_unit_settings_widget.h"

#include <QApplication>
#include <QPushButton>

MonitorUnitSettingsWidget::MonitorUnitSettingsWidget(view_widget::Canvas* parent)
    : QDialog(parent)
    , ui(new Ui::MonitorUnitSettingsWidget)
{
    CommonSetUp();

    ViewWidgetSetUp();

    TransferWidgetSetUp();

    DataStorageWidgetSetUp();

    Resize();
}

MonitorUnitSettingsWidget::MonitorUnitSettingsWidget(
        const app::MonitorUnitSettings& mon_unit_settings,
            view_widget::Canvas* view ,view_widget::Canvas* parent)
    : mu_settings_{mon_unit_settings}
    , QDialog(parent)
    , ui(new Ui::MonitorUnitSettingsWidget) {
    CommonSetUp();

    ViewWidgetSetUp(true, view);

    TransferWidgetSetUp(true);

    DataStorageWidgetSetUp(true);

    Resize();
}

MonitorUnitSettingsWidget::~MonitorUnitSettingsWidget() {
    delete ui;
}

const app::MonitorUnitSettings MonitorUnitSettingsWidget::GetSettings() const {
    return mu_settings_;
}

view_widget::Canvas* MonitorUnitSettingsWidget::GetWidget() const {
    return view_widget_->GetCanvas();
}

void MonitorUnitSettingsWidget::Resize() {
    QScreen *screen = QApplication::primaryScreen();
    QRect screenGeometry = screen->geometry();
    resize(screenGeometry.width() * 0.3, screenGeometry.height() * 0.4);
}

void MonitorUnitSettingsWidget::ValidateAndAccept() {
    if(transfer_stg_wgt_->GetSettings()) {
        mu_settings_.transfer = *(transfer_stg_wgt_->GetSettings());
    } else {
        ui->tabWidget->setCurrentWidget(ui->transfer_tab);
        return;
    }
    if(storage_stg_wgt_->GetSettings()) {
        mu_settings_.data_storage = *(storage_stg_wgt_->GetSettings());
    } else {
        ui->tabWidget->setCurrentWidget(ui->storage_tab);
        return;
    }
    accept();
}

void MonitorUnitSettingsWidget::CommonSetUp() {
    ui->setupUi(this);
    setWindowTitle("Monitor Unit Settings");
    setModal(true);
    disconnect(ui->buttonBox, &QDialogButtonBox::accepted, 0, 0);
    disconnect(ui->buttonBox->button(QDialogButtonBox::Ok), &QPushButton::clicked, 0, 0);
    connect(ui->buttonBox->button(QDialogButtonBox::Ok), &QPushButton::clicked,
            this, &MonitorUnitSettingsWidget::ValidateAndAccept);
}

void MonitorUnitSettingsWidget::ViewWidgetSetUp(bool is_need_settings_setup,
                                            view_widget::Canvas* view) {
    view_widget_ = new view_widget::ViewWidget(ui->view_tab);
    if(is_need_settings_setup) {
        if(view == nullptr) {
            Q_ASSERT("MonitorUnitSettingsWidget::ViewWidgetSetUp: view is nullptr");
        }
        view_widget_->SetUp(view);
    }
    QVBoxLayout* view_layout = new QVBoxLayout(ui->view_tab);
    view_layout->addWidget(view_widget_);
    ui->view_tab->setLayout(view_layout);
}

void MonitorUnitSettingsWidget::TransferWidgetSetUp(bool is_need_settings_setup) {
    transfer_stg_wgt_ = new TransferSettingsWidget(ui->transfer_tab);
    if(is_need_settings_setup) {
        transfer_stg_wgt_->SetUp(mu_settings_.transfer);
    }
    QVBoxLayout* transfer_layout = new QVBoxLayout(ui->transfer_tab);
    transfer_layout->addWidget(transfer_stg_wgt_);
    transfer_layout->addStretch(1);
    ui->transfer_tab->setLayout(transfer_layout);
}

void MonitorUnitSettingsWidget::DataStorageWidgetSetUp(bool is_need_settings_setup) {
    storage_stg_wgt_ = new DataStorageSettingsWidget(ui->storage_tab);
    if(is_need_settings_setup) {
        storage_stg_wgt_->SetUp(mu_settings_.data_storage);
    }
    QVBoxLayout* storage_layout = new QVBoxLayout(ui->storage_tab);
    storage_layout->addWidget(storage_stg_wgt_);
    ui->storage_tab->setLayout(storage_layout);
}
