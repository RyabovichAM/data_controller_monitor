#include "mdisubwindow_decorator.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QLayout>

#include "charts/charts_widget.h"
#include "monitor_unit_settings_widget.h"
#include "view_widget/component_object_widgets/component_objects_widgets.h"

MdiSubWindowDecorator::MdiSubWindowDecorator(app::Application& app, QWidget* parent)
    :   app_{app}
    , QMdiSubWindow{parent}
    , menu_bar_{new QMenuBar(this)} {
    setAttribute(Qt::WA_DeleteOnClose);

    QMenu *fileMenu = menu_bar_->addMenu("File");
    fileMenu->addAction("Close", this, &QMdiSubWindow::close);
    fileMenu->addAction("Settings", this, [self = this]() {
        MonitorUnitSettingsWidget musw(self->MonitorUnit_iter_->Settings(),
                                       self->view_);
        if (musw.exec() == QDialog::Rejected) {
            return;
        }
        self->MonitorUnit_iter_->SetSettings(musw.GetSettings());
        self->MonitorUnit_iter_->SetName(musw.GetWidget()->GetLabel());
        self->setWindowTitle(musw.GetWidget()->GetLabel());
        self->SetWidget(musw.GetWidget());
    });

    QMenu *chartMenu = menu_bar_->addMenu("Chart");
    chartMenu->addAction("Chart", this, [self = this]() {
        ChartsWidget* charts =
            new ChartsWidget{self->MonitorUnit_iter_->DataStorage(),self->view_->GetLabel()};
        charts->setAttribute(Qt::WA_DeleteOnClose);
        charts->show();
    });

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

void MdiSubWindowDecorator::SetWidget(view_widget::Canvas* wgt) {
    if (widget()) {
        QWidget* oldWidget = widget();
        oldWidget->setParent(nullptr);
    }
    wgt->setParent(nullptr);
    view_ = wgt;
    if(!widget()) {
        setWidget(view_);
    }
    layout()->addWidget(view_);
    for(auto component_wgt : wgt->findChildren<QWidget*>()) {
        component_wgt->setAttribute(Qt::WA_TransparentForMouseEvents);
    }
    resize(wgt->size());
    view_->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    view_->setFocusPolicy(Qt::NoFocus);
}

view_widget::Canvas* MdiSubWindowDecorator::View() const {
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
            dynamic_cast<ComponentObjectsWidgets::Label*>(wgt)->SetText(param.value().toString());
    }
}
