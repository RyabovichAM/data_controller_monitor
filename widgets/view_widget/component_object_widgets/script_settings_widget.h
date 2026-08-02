#ifndef SCRIPT_SETTINGS_WIDGET_H
#define SCRIPT_SETTINGS_WIDGET_H

#include <QDialog>

namespace Ui {
class ScriptSettingsWidget;
}

namespace ComponentObjectsWidgets {

struct ScriptSettings {
    QString code;
};

class ScriptSettingsWidget : public QDialog
{
    Q_OBJECT

public:
    explicit ScriptSettingsWidget(QWidget *parent = nullptr, 
                                        const QString& code = "");
    ~ScriptSettingsWidget();

    const ScriptSettings& GetSettings() const;

private slots:
    void on_buttonBox_accepted();

    void on_buttonBox_rejected();

private:
    Ui::ScriptSettingsWidget *ui;
    ScriptSettings settings_;

    void SetUp();
};

}   //namespace ComponentObjectsWidgets

#endif // SCRIPT_SETTINGS_WIDGET_H
