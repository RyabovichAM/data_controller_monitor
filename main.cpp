#include "mainwindow.h"

#include <QApplication>

#include "application.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    app::Application app;
    MainWindow mainWindow(app);
    mainWindow.show();
    return a.exec();
}
