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

}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::ClickNewControllerViewer(bool) {
    MdiSubWindowDecorator *subWindow = new MdiSubWindowDecorator;

    MonitorUnitSettingsWidget musw;
    musw.exec();

    auto unit_iter = app_.CreateUnit(musw.GetWidget(),musw.GetSettings());

    subWindow->AddMonitorUnit(unit_iter);
    subWindow->setWidget(musw.GetWidget());

    mdi_area_.addSubWindow(subWindow);
    subWindow->show();
}
