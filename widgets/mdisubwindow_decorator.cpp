#include "mdisubwindow_decorator.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QLayout>

#include "component_widgets.h"

MdiSubWindowDecorator::MdiSubWindowDecorator(app::Application& app, QWidget* parent)
    :   app_{app}
    , QMdiSubWindow{parent}
    , menu_bar_{new QMenuBar(this)} {
    setAttribute(Qt::WA_DeleteOnClose);


    QMenu *fileMenu = menu_bar_->addMenu("File");
    fileMenu->addAction("Close", this, &QMdiSubWindow::close);

    layout()->setMenuBar(menu_bar_);
}

MdiSubWindowDecorator::~MdiSubWindowDecorator() {
    MonitorUnit_iter_->Stop();
    app_.DeleteUnit(MonitorUnit_iter_);
}

void MdiSubWindowDecorator::AddMonitorUnit(const app::MonitorUnit_Iter& iter) {
    MonitorUnit_iter_ = iter;
    MonitorUnit_iter_->Start();
}

void MdiSubWindowDecorator::SetWidget(DropArea* wgt) {
    view_ = wgt;
    setWidget(wgt);
    for(auto component_wgt : wgt->findChildren<QWidget*>()) {
        component_wgt->setAttribute(Qt::WA_TransparentForMouseEvents);
    }
    resize(wgt->size());
}

DropArea* MdiSubWindowDecorator::View() const {
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
    auto json_obj = data.object();
    auto& updateble_wgts_maps = subwindow_->View()->GetUpdatebleWidgets();
    for (auto param = json_obj.begin(); param != json_obj.end(); ++param) {
        auto* wgt = updateble_wgts_maps[param.key()];
        if(wgt != nullptr)
            dynamic_cast<ComponentWidgets::Label*>(wgt)->setText(param.value().toString());
    }
}
