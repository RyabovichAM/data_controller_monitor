#ifndef MDISUBWINDOW_DECORATOR_H
#define MDISUBWINDOW_DECORATOR_H

#include <QMdiSubWindow>

#include "application.h"

class MdiSubWindowDecorator : public QMdiSubWindow
{
public:
    MdiSubWindowDecorator(QWidget* parent = nullptr);
    void AddMonitorUnit(const app::MonitorUnit_Iter& iter);
    void SetWidget(QWidget* wgt);

private:
    app::MonitorUnit_Iter MonitorUnit_iter_;
};

#endif // MDISUBWINDOW_DECORATOR_H
