#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMdiArea>

#include "application.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(app::Application& app,  QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void ClickNewControllerViewer(bool);

private:
    Ui::MainWindow *ui;
    QMdiArea mdi_area_;
    app::Application& app_;
};
#endif // MAINWINDOW_H
