#include "mdisubwindow_decorator.h"

#include <QLayout>

MdiSubWindowDecorator::MdiSubWindowDecorator(QWidget* parent)
    :   QMdiSubWindow{parent}
    , menu_bar_{new QMenuBar(this)} {
    setAttribute(Qt::WA_DeleteOnClose);


    QMenu *fileMenu = menu_bar_->addMenu("Файл");
    fileMenu->addAction("Закрыть", this, &QMdiSubWindow::close);

    layout()->setMenuBar(menu_bar_);
}

void MdiSubWindowDecorator::AddMonitorUnit(const app::MonitorUnit_Iter& iter) {
    MonitorUnit_iter_ = iter;
}

void MdiSubWindowDecorator::SetWidget(QWidget* wgt) {
    setWidget(wgt);
    for(auto component_wgt : wgt->findChildren<QWidget*>()) {
        component_wgt->setAttribute(Qt::WA_TransparentForMouseEvents);
    }
    resize(wgt->size());
}

void MdiSubWindowDecorator::SetMenuAvailable(bool is_avaibality) {
    layout()->menuBar()->setVisible(is_avaibality);
}


