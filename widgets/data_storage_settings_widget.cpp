#include "data_storage_settings_widget.h"
#include "ui_data_storage_settings_widget.h"

#include <QFileDialog>
#include <QMessageBox>

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

std::optional<QHash<QString, QString>> DataStorageSettingsWidget::GetSettings() {
    if(ui->data_storage_enable_check->checkState() == Qt::Unchecked)
        return QHash<QString, QString>{{"is_enable", "not_enable"}};

    if (ui->location->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "Error", "Field cannot be empty");
        ui->location->setFocus();
        return std::nullopt;
    }
    if (ui->period->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "Error", "Field cannot be empty");
        ui->period->setFocus();
        return std::nullopt;
    }
    QHash<QString, QString> settings;
    settings["is_enable"] = "enable";
    settings["type"] = "file";
    settings["location"] = ui->location->text();
    settings["period"] = ui->period->text();
    settings["data_format"] = ui->is_binary-> isChecked() ? "binary" : "text";

    return settings;
}

void DataStorageSettingsWidget::OnBrowseDirClicked() {
    QString dir = QFileDialog::getExistingDirectory(
        this,
        tr("Select a Directory"),
        QString(),
        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks
        );

    if (!dir.isEmpty()) {
        ui->location->setText(dir);
    }
}
