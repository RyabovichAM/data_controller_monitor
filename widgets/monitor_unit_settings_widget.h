#ifndef MONITOR_UNIT_SETTINGS_WIDGET_H
#define MONITOR_UNIT_SETTINGS_WIDGET_H

#include <QDialog>

#include "app_domain.h"
#include "data_storage_settings_widget.h"
#include "transfer_settings_widget.h"
#include "view_widget/view_widget.h"

namespace Ui {
class MonitorUnitSettingsWidget;
}

class MonitorUnitSettingsWidget : public QDialog {
    Q_OBJECT

public:
    explicit MonitorUnitSettingsWidget(view_widget::Canvas* parent = nullptr);
    explicit MonitorUnitSettingsWidget(
        const app::MonitorUnitSettings& mon_unit_settings,
        view_widget::Canvas* view ,view_widget::Canvas* parent = nullptr);
    ~MonitorUnitSettingsWidget();

    const app::MonitorUnitSettings GetSettings() const;
    view_widget::Canvas* GetWidget() const;
    void Resize();

public slots:
    void ValidateAndAccept();

private:
    Ui::MonitorUnitSettingsWidget *ui;
    app::MonitorUnitSettings mu_settings_;

    view_widget::ViewWidget* view_widget_{nullptr};
    TransferSettingsWidget* transfer_stg_wgt_{nullptr};
    DataStorageSettingsWidget* storage_stg_wgt_{nullptr};

    bool is_need_settings_setup_;
    void CommonSetUp();
    void ViewWidgetSetUp(bool is_need_settings_setup = false,
                         view_widget::Canvas* view = nullptr);
    void TransferWidgetSetUp(bool is_need_settigs_setup = false);
    void DataStorageWidgetSetUp(bool is_need_settings_setup = false);
};

#endif // MONITOR_UNIT_SETTINGS_WIDGET_H
