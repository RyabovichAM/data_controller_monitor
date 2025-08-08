#include "label_settings_widget.h"
#include "ui_label_settings_widget.h"

namespace ComponentWidgets {

LabelSettingsWidget::LabelSettingsWidget(QWidget *parent, const QString& label,
                                 const QString& obj_name, QSize size)
    : QDialog(parent)
    , ui(new Ui::LabelSettingsWidget)
{
    ui->setupUi(this);

    ui->label_edit->setText(label);
    ui->obj_name_edit->setText(obj_name);
    ui->height_edit->setText(QString::number(size.height()));
    ui->width_edit->setText(QString::number(size.width()));
}

LabelSettingsWidget::~LabelSettingsWidget()
{
    delete ui;
}

void LabelSettingsWidget::on_buttonBox_accepted()
{
    settings_.label = ui->label_edit->text();
    settings_.object_name = ui->obj_name_edit->text();
    settings_.width = ui->width_edit->text().toInt();
    settings_.height = ui->height_edit->text().toInt();
}

const LabelSettings& LabelSettingsWidget::GetSettings() const {
    return settings_;
}

}   //namespace ComponentWidgets
