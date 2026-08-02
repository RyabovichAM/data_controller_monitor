#include "script_settings_widget.h"
#include "ui_script_settings_widget.h"

namespace ComponentObjectsWidgets {

ScriptSettingsWidget::ScriptSettingsWidget(QWidget *parent, const QString& code)
    : QDialog(parent)
    , ui(new Ui::ScriptSettingsWidget) {
    ui->setupUi(this);

    ui->script_edit->setHtml(code);
    //
    SetUp();
}

ScriptSettingsWidget::~ScriptSettingsWidget() {
    delete ui;
}

void ScriptSettingsWidget::on_buttonBox_accepted() {
    SetUp();
}

void ScriptSettingsWidget::on_buttonBox_rejected() {
    close();
}

const ScriptSettings& ScriptSettingsWidget::GetSettings() const {
    return settings_;
}

void ScriptSettingsWidget::SetUp() {
    settings_.code = ui->script_edit->toPlainText();
}

}   //namespace ComponentObjectsWidgets