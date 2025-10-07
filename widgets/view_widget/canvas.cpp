#include "canvas.h"

#include <QApplication>

#include <QPainter>
#include <QPaintEvent>
#include <QPen>

#include "component_widgets.h"

namespace view_widget {

QString Shape::TypeToString() {
    switch (tool_type) {
    case ComponentWidgetIndex::Label:
        return "Label";
    case ComponentWidgetIndex::Rectangle:
        return "Rectangle";
    case ComponentWidgetIndex::Ellipse:
        return "Ellipse";
    case ComponentWidgetIndex::Line:
        return "Line";
    case ComponentWidgetIndex::Brush:
        return "Brush";
    default:
        Q_ASSERT("View_widget: Shape::TypeToString - unprocessed type");
    }
    return {};
}

Canvas::Canvas(QWidget* parent) : QFrame(parent) {
    setAcceptDrops(true);
    setStyleSheet("background-color: white; border: 2px dashed #aaa;");

    QScreen *screen = QApplication::primaryScreen();
    QRect screenGeometry = screen->geometry();
    setMinimumSize(screenGeometry.width() * 0.3, screenGeometry.height() * 0.4);
}

ValueUpdatedWidgetsByObjName& Canvas::GetUpdatebleWidgets() {
    return value_updated_widgets_by_obj_name_;
}

QList<view_widget::Shape>& Canvas::GetShapes() {
    return shapes_;
}

void Canvas::SetCurrentComponentWidgetIndex(ComponentWidgetIndex idx) {
    current_tool_ = idx;
}

void Canvas::DeleteShapeByIndex(int index) {
    shapes_.removeAt(index);
    update();
}

void Canvas::dragEnterEvent(QDragEnterEvent* event) {
    event->acceptProposedAction();
}

void Canvas::dropEvent(QDropEvent* event)  {
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
        if(dynamic_cast<ComponentWidgets::Label*>(component_widget)) {
            auto lbl_ptr = dynamic_cast<ComponentWidgets::Label*>(component_widget);
            ComponentWidgets::CW_ObserverBase* observer = new ComponentWidgets::CW_ObserverBase{component_widget};
            observer->SetOnObjectNameChanged([self = this, component_widget](const QString& prev_obj_name, const QString& obj_name){
                self->value_updated_widgets_by_obj_name_.remove(prev_obj_name);
                self->value_updated_widgets_by_obj_name_.insert(obj_name,component_widget);
            });
            lbl_ptr->SetObserver(observer);
        }
    }

    component_widget->move(event->position().toPoint() - QPoint(component_widget->width()/2, component_widget->height()/2));
    component_widget->show();
    event->accept();
}

void Canvas::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        if(current_tool_ == ComponentWidgetIndex::Label) {
            ComponentWidgets::Label* label =  new ComponentWidgets::Label{this};
            label->move(event->position().toPoint() - QPoint(label->width()/2, label->height()/2));
            label->show();
            event->accept();
            return;
        }

        drawing_ = true;
        current_shape_.tool_type = current_tool_;
        current_shape_.color = QColor{};
        current_shape_.points.clear();
        current_shape_.points.append(event->pos());
        current_shape_.points.append(event->pos());
    }
}

void Canvas::mouseMoveEvent(QMouseEvent *event) {
    if (drawing_ && (event->buttons() & Qt::LeftButton)) {
        if(current_shape_.tool_type == ComponentWidgetIndex::Brush) {
            current_shape_.points.append(event->pos());
        } else {
            current_shape_.points.last() = event->pos();
        }
        update();
    }
}

void Canvas::mouseReleaseEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton && drawing_) {
        drawing_ = false;
        shapes_.append(current_shape_);
        update();
    }
    emit FigureAdded(current_shape_.TypeToString());
}

void Canvas::paintEvent(QPaintEvent *event) {
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);


    for (const view_widget::Shape& shape : shapes_) {
        painter.setPen(QPen(shape.color, 2));
        painter.setBrush(QBrush(shape.color, Qt::NoBrush));

        DrawShape(shape, painter);
    }

    if (drawing_) {
        painter.setPen(QPen(current_shape_.color, 2, Qt::DashLine));
        DrawShape(current_shape_, painter);
    }
}

void Canvas::DrawShape(const view_widget::Shape& shape, QPainter& painter) {
    switch (shape.tool_type) {
    case ComponentWidgetIndex::Rectangle:
        painter.drawRect(QRect(shape.points.first(),
                               shape.points.last()).normalized());
        break;
    case ComponentWidgetIndex::Ellipse:
        painter.drawEllipse(QRect(shape.points.first(),
                                  shape.points.last()).normalized());
        break;
    case ComponentWidgetIndex::Line:
        painter.drawLine(shape.points.first(),
                         shape.points.last());
        break;
    case ComponentWidgetIndex::Brush:
        for (int i = 1; i < shape.points.size(); ++i) {
            painter.drawLine(shape.points[i-1], shape.points[i]);
        }
        break;
    case ComponentWidgetIndex::Label:
        Q_ASSERT("Label no drawable");
    }
}

}   //view_widget
