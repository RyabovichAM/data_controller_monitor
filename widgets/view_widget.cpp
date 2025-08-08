#include "view_widget.h"

#include <QMimeData>
#include <QMouseEvent>
#include <QVBoxLayout>

#include "component_widgets.h"

DragArea::DragArea(QWidget* parent) : QFrame(parent) {
    setStyleSheet("border: 2px dashed #aaa;");

    QVBoxLayout* main_layout = new QVBoxLayout{this};
    main_layout->addWidget(new ComponentWidgets::Label{this});
    main_layout->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Expanding));

    setLayout(main_layout);

}

DropArea::DropArea(QWidget* parent) : QFrame(parent) {
    setAcceptDrops(true);
    setStyleSheet("background-color: white; border: 2px dashed #aaa;");
}

void DropArea::dragEnterEvent(QDragEnterEvent* event) {
    event->acceptProposedAction();
}

void DropArea::dropEvent(QDropEvent* event)  {
    QString text = event->mimeData()->text();
    quintptr ptr = text.toULongLong();

    QWidget* component_widget_to_check = reinterpret_cast<QWidget*>(ptr);
    if(component_widget_to_check == nullptr)
        return;

    QWidget* component_widget;
    if(component_widget_to_check->parent() == this) {
        component_widget = component_widget_to_check;
    } else {
        component_widget = ComponentWidgets::CurrentComponentByPtr(component_widget_to_check,this);
    }

    component_widget->move(event->position().toPoint() - QPoint(component_widget->width()/2, component_widget->height()/2));
    component_widget->show();
    event->accept();
}

ViewWidget::ViewWidget(QWidget *parent)
    : QWidget{parent}
{
    setWindowTitle("View Widget");
    setGeometry(100, 100, 800, 600);

    QHBoxLayout* layout = new QHBoxLayout{};
    setLayout(layout);

    dragArea = new DragArea{this};
    layout->addWidget(dragArea,1);

    dropArea = new DropArea{this};
    layout->addWidget(dropArea,4);
}
