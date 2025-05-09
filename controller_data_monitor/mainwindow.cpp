#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include <QMdiSubWindow>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , mdi_area_{}
{
    ui->setupUi(this);
    setCentralWidget(&mdi_area_);

    connect(ui->actionNew_ControllerViewer, &QAction::triggered, this, &MainWindow::ClickNewControllerViewer);

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::ClickNewControllerViewer(bool) {
    QMdiSubWindow *subWindow = new QMdiSubWindow(this);

    mdi_area_.addSubWindow(subWindow);
    subWindow->show();
}
