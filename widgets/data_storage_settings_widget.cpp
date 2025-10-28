#include "data_storage_settings_widget.h"
#include "ui_data_storage_settings_widget.h"

#include <QFileDialog>

DataStorageSettingsWidget::DataStorageSettingsWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::DataStorageSettingsWidget) {
    ui->setupUi(this);
    connect(ui->get_dir_btn, &QPushButton::clicked,
            this, & DataStorageSettingsWidget::OnBrowseDirClicked);
}

DataStorageSettingsWidget::~DataStorageSettingsWidget() {
    delete ui;
}

void DataStorageSettingsWidget::on_data_storage_enable_check_stateChanged(int arg1) {
    ui->groupBox->setEnabled(arg1 == Qt::Checked);
}

QHash<QString, QString> DataStorageSettingsWidget::GetSettings() {
    if(ui->data_storage_enable_check->checkState() == Qt::Unchecked)
        return {{"is_enable","not_enable"}};

    QHash<QString, QString> settings;
    settings["is_enable"] = "enable";
    settings["type"] = "csv";
    settings["location"] = ui->location->text();
    settings["period"] = ui->period->text();

    return settings;
}

void DataStorageSettingsWidget::OnBrowseDirClicked() {
    QString dir = QFileDialog::getExistingDirectory(
        this,
        tr("Выбор директории"),
        QString(),
        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks
        );

    if (!dir.isEmpty()) {
        ui->location->setText(dir);
        qDebug() << "Выбрана директория:" << dir;
    }
}
