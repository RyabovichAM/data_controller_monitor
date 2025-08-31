#ifndef MDISUBWINDOW_DECORATOR_H
#define MDISUBWINDOW_DECORATOR_H

#include <QMdiSubWindow>
#include <QMenuBar>

#include "application.h"
#include "view_widget.h"

class MdiSubWindowDecorator : public QMdiSubWindow
{
public:
    MdiSubWindowDecorator(app::Application& app, QWidget* parent = nullptr);
    void AddMonitorUnit(const app::MonitorUnit_Iter& iter);
    void SetWidget(DropArea* wgt);
    DropArea* View() const;
    void SetMenuAvailable(bool is_avaibality = true);
    ~MdiSubWindowDecorator();

private:
    app::Application& app_;
    app::MonitorUnit_Iter MonitorUnit_iter_;
    QMenuBar* menu_bar_{nullptr};
    DropArea* view_{nullptr};
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
