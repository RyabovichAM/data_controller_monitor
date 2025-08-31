#include "mainwindow.h"
#include "./ui_mainwindow.h"


#include "monitor_unit_settings_widget.h"
#include "mdisubwindow_decorator.h"

MainWindow::MainWindow(app::Application& app, QWidget *parent)
    : QMainWindow(parent)
    , app_{app}
    , ui(new Ui::MainWindow)
    , mdi_area_{}
{
    ui->setupUi(this);
    setCentralWidget(&mdi_area_);

    connect(ui->actionNew_ControllerViewer, &QAction::triggered, this, &MainWindow::ClickNewControllerViewer);
    connect(ui->actionEdit, &QAction::triggered, this, &MainWindow::ClickEditMode);
    connect(ui->actionView, &QAction::triggered, this, &MainWindow::ClickViewMode);
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::ClickNewControllerViewer(bool) {
    MdiSubWindowDecorator* subWindow = new MdiSubWindowDecorator{app_};

    MonitorUnitSettingsWidget musw;
    musw.exec();

    SubWindow_MU_observer* subWindow_observer = new SubWindow_MU_observer(subWindow,subWindow);

    auto unit_iter = app_.CreateUnit(musw.GetSettings());
    unit_iter->SetObserver(subWindow_observer);

    subWindow->AddMonitorUnit(unit_iter);
    subWindow->SetWidget(musw.GetWidget());

    mdi_area_.addSubWindow(subWindow);
    subWindow->show();
}

void MainWindow::ClickEditMode(bool) {
    for(auto& subwindow : mdi_area_.subWindowList()) {
        auto subwindow_decor = static_cast<MdiSubWindowDecorator*>(subwindow);
        subwindow_decor->SetMenuAvailable(true);
        subwindow_decor->setWindowFlags(Qt::Widget);
    }
}

void MainWindow::ClickViewMode(bool) {
    for(auto& subwindow : mdi_area_.subWindowList()) {
        auto subwindow_decor = static_cast<MdiSubWindowDecorator*>(subwindow);
        subwindow_decor->SetMenuAvailable(false);
        subwindow_decor->setWindowFlags(Qt::Window | Qt::WindowTitleHint);
    }
}
