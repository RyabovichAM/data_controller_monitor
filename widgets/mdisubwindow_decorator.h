#ifndef MDISUBWINDOW_DECORATOR_H
#define MDISUBWINDOW_DECORATOR_H

#include <memory>
#include <QMdiSubWindow>
#include <QMenuBar>

#include "application.h"
#include "data_storage_interface.h"
#include "view_widget/view_widget.h"

class MdiSubWindowDecorator : public QMdiSubWindow
{
public:
    using DataStorageSaveType = QString;
    using DataStorageLoadType = QList<QPair<QDateTime,QJsonDocument>>;
    using DataStorage = data_storage::DataStorageInterface
                            <DataStorageSaveType,DataStorageLoadType>;

    MdiSubWindowDecorator(app::Application& app, QWidget* parent = nullptr);
    void AddMonitorUnit(const app::MonitorUnit_Iter& iter);
    void SetWidget(view_widget::Canvas* wgt);
    view_widget::Canvas* View() const;
    DataStorage* Storage() const;
    void SetMenuAvailable(bool is_avaibality = true);
    ~MdiSubWindowDecorator();

private:
    app::Application& app_;
    app::MonitorUnit_Iter MonitorUnit_iter_;
    QMenuBar* menu_bar_{nullptr};
    view_widget::Canvas* view_{nullptr};
    std::unique_ptr<DataStorage> data_storage_;

    void InitDataSaving();
    void DeinitDataSaving();
};

class SubWindow_MU_observer : public app::MU_ObserverBase {
public:
    SubWindow_MU_observer(MdiSubWindowDecorator* subwindow, QObject* parent = nullptr);

    void Update(const QJsonDocument& data) override;
    virtual ~SubWindow_MU_observer() = default;

private:
    MdiSubWindowDecorator* subwindow_;
};
#endif // MDISUBWINDOW_DECORATOR_H
