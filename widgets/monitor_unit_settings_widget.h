#ifndef MONITOR_UNIT_SETTINGS_WIDGET_H
#define MONITOR_UNIT_SETTINGS_WIDGET_H

#include <QDialog>

#include "app_domain.h"
#include "transfer_settings_widget.h"
#include "view_widget.h"

namespace Ui {
class MonitorUnitSettingsWidget;
}

class MonitorUnitSettingsWidget : public QDialog
{
    Q_OBJECT

public:
    explicit MonitorUnitSettingsWidget(QWidget *parent = nullptr);
    ~MonitorUnitSettingsWidget();

    const app::MonitorUnitSettings GetSettings() const;
    QWidget* GetWidget() const;

private:
    Ui::MonitorUnitSettingsWidget *ui;

    TransferSettingsWidget* transfer_stg_wgt_;
    ViewWidget* view_widget_;
};

#endif // MONITOR_UNIT_SETTINGS_WIDGET_H
