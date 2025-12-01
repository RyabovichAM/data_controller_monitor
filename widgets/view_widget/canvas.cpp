#include "canvas.h"

#include <QPainter>
#include <QPaintEvent>
#include <QPen>

namespace view_widget {

Canvas::Canvas(QWidget* parent) : QFrame(parent) {
    setAcceptDrops(true);
    setStyleSheet("background-color: white; border: 2px solid grey;");
}

ValueUpdatedWidgetsByObjName& Canvas::GetUpdatebleWidgets() {
    return value_updated_widgets_by_obj_name_;
}

QList<view_widget::Shape>& Canvas::GetShapes() {
    return shapes_;
}

QList<ComponentObjectsWidgets::ComponentObjectWgtInterface*> Canvas::GetObjects() {
    QList<ComponentObjectsWidgets::ComponentObjectWgtInterface*> children_wgts;

    const QObjectList& children = this->children();

    for (QObject* obj : children) {
        if (ComponentObjectsWidgets::ComponentObjectWgtInterface* widget =
            dynamic_cast<ComponentObjectsWidgets::ComponentObjectWgtInterface*>(obj)) {
            children_wgts.append(widget);
        }
    }

    return children_wgts;
}

void Canvas::SetLabel(const QString& label) {
    label_ = label;
}

const QString Canvas::GetLabel() const {
    return label_;
}

void Canvas::SetCurrentComponentWidgetType(ComponentWidgetType type,
                                   const QString& curr_str_sub_type) {
    current_com_wgt_type_ = type;
    current_component_sub_type_str_ = curr_str_sub_type;
}

void Canvas::DeleteShapeByIndex(int index) {
    shapes_.removeAt(index);
    update();
}

void Canvas::DeleteObjectByIndex(int index) {
    QObjectList children = this->children();
    if (index < 0 || index >= children.size()) {
        return;
    }

    QObject* child = children.at(index);
    if(auto wgt = static_cast<ComponentObjectsWidgets::ComponentObjectWgtInterface*>(child)) {
        value_updated_widgets_by_obj_name_.remove(wgt->objectName());
        wgt->close();
    }
}

void Canvas::dragEnterEvent(QDragEnterEvent* event) {
    event->acceptProposedAction();
}

void Canvas::dropEvent(QDropEvent* event)  {
    QString text = event->mimeData()->text();
    quintptr ptr = text.toULongLong();

    QWidget* component_widget = reinterpret_cast<QWidget*>(ptr);
    if(component_widget == nullptr)
        return;

    component_widget->move(event->position().toPoint() -
                           QPoint(component_widget->width()/2, component_widget->height()/2));
    component_widget->show();
    event->accept();
}

void Canvas::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        if(current_com_wgt_type_ == ComponentWidgetType::Oblect) {
            ComponentObjectsWidgets::ComponentObjectWgtInterface* obj_wgt =
                ComponentObjectsWidgets::MakeComponentObjectsWgt(
                ComponentObjectsWidgets::ComponentObjectsStringToType(
                                    current_component_sub_type_str_),this);

            ComponentObjectsWidgets::COW_ObserverBase* observer =
                        new ComponentObjectsWidgets::COW_ObserverBase{obj_wgt};
            observer->SetOnObjectNameChanged([self = this, obj_wgt](
                                        const QString& prev_obj_name, const QString& obj_name){
                self->value_updated_widgets_by_obj_name_.remove(prev_obj_name);
                self->value_updated_widgets_by_obj_name_.insert(obj_name,obj_wgt);
            });
            obj_wgt->SetObserver(observer);

            obj_wgt->move(event->position().toPoint());
            obj_wgt->show();
            event->accept();
            emit ItemAdded(current_com_wgt_type_,current_component_sub_type_str_);
            return;
        }

        if(current_com_wgt_type_ == ComponentWidgetType::Shape) {
            drawing_ = true;
            current_shape_.tool_type = view_widget::Shape::StringToType(
                                                    current_component_sub_type_str_);
            current_shape_.color = QColor{};
            current_shape_.points.clear();
            current_shape_.points.append(event->pos());
            current_shape_.points.append(event->pos());
        }
    }
}

void Canvas::mouseMoveEvent(QMouseEvent *event) {
    if (drawing_ && (event->buttons() & Qt::LeftButton)) {
        if(current_shape_.tool_type == view_widget::Shape::Type::Brush) {
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
        emit ItemAdded(current_com_wgt_type_,current_component_sub_type_str_);
        update();
    }
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
    case view_widget::Shape::Type::Rectangle:
        painter.drawRect(QRect(shape.points.first(),
                               shape.points.last()).normalized());
        break;
    case view_widget::Shape::Type::Ellipse:
        painter.drawEllipse(QRect(shape.points.first(),
                                  shape.points.last()).normalized());
        break;
    case view_widget::Shape::Type::Line:
        painter.drawLine(shape.points.first(),
                         shape.points.last());
        break;
    case view_widget::Shape::Type::Brush:
        for (int i = 1; i < shape.points.size(); ++i) {
            painter.drawLine(shape.points[i-1], shape.points[i]);
        }
        break;
    default:
        Q_ASSERT("Canvas::DrawShape: no drawable item");
    }
}

}   //view_widget
