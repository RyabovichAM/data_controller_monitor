#ifndef LABEL_SETTINGS_WIDGET_H
#define LABEL_SETTINGS_WIDGET_H

#include <QDialog>

namespace Ui {
class LabelSettingsWidget;
}

namespace ComponentWidgets {

struct LabelSettings {
    QString label;
    QString object_name;
    size_t width;
    size_t height;
};

class LabelSettingsWidget : public QDialog
{
    Q_OBJECT

public:
    explicit LabelSettingsWidget(QWidget *parent = nullptr, const QString& label = "",
                                 const QString& obj_name = "", QSize size = {100,100});
    ~LabelSettingsWidget();

    const LabelSettings& GetSettings() const;

private slots:
    void on_buttonBox_accepted();

    void on_buttonBox_rejected();

private:
    Ui::LabelSettingsWidget *ui;
    LabelSettings settings_;

    void SetUp();
};

}   //namespace ComponentWidgets

#endif // LABEL_SETTINGS_WIDGET_H
