#ifndef MONITOR_UNIT_SETTINGS_WIDGET_H
#define MONITOR_UNIT_SETTINGS_WIDGET_H

#include <QDialog>

#include "transfer_settings_widget.h"

namespace Ui {
class MonitorUnitSettingsWidget;
}

class MonitorUnitSettingsWidget : public QDialog
{
    Q_OBJECT

public:
    explicit MonitorUnitSettingsWidget(QWidget *parent = nullptr);
    ~MonitorUnitSettingsWidget();

private:
    Ui::MonitorUnitSettingsWidget *ui;

    TransferSettingsWidget* transfer_stg_wgt_;
};

#endif // MONITOR_UNIT_SETTINGS_WIDGET_H
