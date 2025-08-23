#include "mdisubwindow_decorator.h"

#include <QLayout>

MdiSubWindowDecorator::MdiSubWindowDecorator(QWidget* parent)
    :   QMdiSubWindow{parent}
    , menu_bar_{new QMenuBar(this)} {
    setAttribute(Qt::WA_DeleteOnClose);


    QMenu *fileMenu = menu_bar_->addMenu("File");
    fileMenu->addAction("Close", this, &QMdiSubWindow::close);

    layout()->setMenuBar(menu_bar_);
}

void MdiSubWindowDecorator::AddMonitorUnit(const app::MonitorUnit_Iter& iter) {
    MonitorUnit_iter_ = iter;
}

void MdiSubWindowDecorator::SetWidget(DropArea* wgt) {
    view_ = wgt;
    setWidget(wgt);
    for(auto component_wgt : wgt->findChildren<QWidget*>()) {
        component_wgt->setAttribute(Qt::WA_TransparentForMouseEvents);
    }
    resize(wgt->size());
}

QWidget* MdiSubWindowDecorator::View() const {
    return view_;
}

void MdiSubWindowDecorator::SetMenuAvailable(bool is_avaibality) {
    layout()->menuBar()->setVisible(is_avaibality);
}

SubWindow_MU_observer::SubWindow_MU_observer(MdiSubWindowDecorator* subwindow, QObject* parent)
    :   subwindow_{subwindow},
        MU_ObserverBase{parent} {

}

void SubWindow_MU_observer::Update(const QJsonDocument& data) {
    //TODO
}
