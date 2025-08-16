#include "mdisubwindow_decorator.h"

#include <QMenuBar>
#include <QLayout>

MdiSubWindowDecorator::MdiSubWindowDecorator(QWidget* parent)
    :   QMdiSubWindow{parent} {
    setAttribute(Qt::WA_DeleteOnClose);

    QMenuBar *menuBar = new QMenuBar(this);
    QMenu *fileMenu = menuBar->addMenu("Файл");
    fileMenu->addAction("Закрыть", this, &QMdiSubWindow::close);

    this->layout()->setMenuBar(menuBar);
}

void MdiSubWindowDecorator::SetWidget(QWidget* wgt) {
    setWidget(wgt);
    for(auto component_wgt : wgt->findChildren<QWidget*>()) {
        component_wgt->setAttribute(Qt::WA_TransparentForMouseEvents);
    }
    resize(wgt->size());
}


void MdiSubWindowDecorator::AddMonitorUnit(const app::MonitorUnit_Iter& iter) {
    MonitorUnit_iter_ = iter;
}
