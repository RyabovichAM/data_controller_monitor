#include "mdisubwindow_decorator.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QLayout>
#include <QMessageBox>

#include "charts/charts_widget.h"
#include "data_storage_factory.h"
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
            new ChartsWidget{self->Storage(), self->view_->GetLabel()};
        charts->setAttribute(Qt::WA_DeleteOnClose);
        charts->show();
    });

    layout()->setMenuBar(menu_bar_);
}

MdiSubWindowDecorator::~MdiSubWindowDecorator() {
    MonitorUnit_iter_->Stop();
    DeinitDataSaving();
    app_.DeleteUnit(MonitorUnit_iter_);
}

void MdiSubWindowDecorator::AddMonitorUnit(const app::MonitorUnit_Iter& iter) {
    MonitorUnit_iter_ = iter;
    MonitorUnit_iter_->SetErrorHandler([](const QString& error) {
        QMessageBox::warning(nullptr, "Transmission Error", error);
    });
    InitDataSaving();
    MonitorUnit_iter_->Start();
}

void MdiSubWindowDecorator::InitDataSaving() {
    QHash<QString,QString> settings = MonitorUnit_iter_->Settings().data_storage;
    if(settings["is_enable"] == "not_enable") {
        return;
    }

    settings["location"] += MonitorUnit_iter_->Name() + "/";
    data_storage_ = data_storage::DataStorageFactory::CreateDataStorage
                        <DataStorageSaveType,DataStorageLoadType>(settings);
    data_storage_->SetErrorHandler([](const QString& error) {
        QMessageBox::warning(nullptr, "DataStorage Error", error);
    });
    data_storage_->Open();
}

void MdiSubWindowDecorator::DeinitDataSaving() {
    if(data_storage_) {
        data_storage_->Close();
        data_storage_.reset();
    }
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

MdiSubWindowDecorator::DataStorage* MdiSubWindowDecorator::Storage() const {
    return data_storage_.get();
}

void MdiSubWindowDecorator::SetMenuAvailable(bool is_avaibality) {
    layout()->menuBar()->setVisible(is_avaibality);
}

SubWindow_MU_observer::SubWindow_MU_observer(MdiSubWindowDecorator* subwindow, QObject* parent)
    :   subwindow_{subwindow},
        MU_ObserverBase{parent} {

}

void SubWindow_MU_observer::Update(const QJsonDocument& data) {
    if(auto* storage = subwindow_->Storage()) {
        storage->DataSave(data.toJson(QJsonDocument::Compact));
    }

    auto json_obj = data.object();
    auto& updateble_wgts_maps = subwindow_->View()->GetUpdatebleWidgets();
    for (auto param = json_obj.begin(); param != json_obj.end(); ++param) {
        auto* wgt = updateble_wgts_maps[param.key()];
        if(wgt != nullptr)
            dynamic_cast<ComponentObjectsWidgets::Label*>(wgt)->SetText(param.value().toString());
    }
}
