#ifndef MDISUBWINDOW_DECORATOR_H
#define MDISUBWINDOW_DECORATOR_H

#include <QMdiSubWindow>
#include <QMenuBar>

#include "application.h"

class MdiSubWindowDecorator : public QMdiSubWindow
{
public:
    MdiSubWindowDecorator(QWidget* parent = nullptr);
    void AddMonitorUnit(const app::MonitorUnit_Iter& iter);
    void SetWidget(QWidget* wgt);
    void SetMenuAvailable(bool is_avaibality = true);

private:
    app::MonitorUnit_Iter MonitorUnit_iter_;
    QMenuBar* menu_bar_;
};

#endif // MDISUBWINDOW_DECORATOR_H
