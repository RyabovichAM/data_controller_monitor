#include "view_widget.h"

#include <QMimeData>

namespace view_widget {

LayoutEventFilter::LayoutEventFilter(QVBoxLayout* layout, QObject* parent)
    : QObject(parent), layout_(layout) {
    for (int i = 0; i < layout_->count(); ++i) {
        QWidget* widget = layout_->itemAt(i)->widget();
        if (widget) {
            widget->installEventFilter(this);
        }
    }
    SetDefaultStyle();
}

bool LayoutEventFilter::eventFilter(QObject* obj, QEvent* event) {
    if (event->type() == QEvent::MouseButtonPress) {
        QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
        if (mouseEvent->button() == Qt::LeftButton) {
            QWidget* widget = qobject_cast<QWidget*>(obj);
            if (widget) {
                HandleWidgetClick(widget);
                return true;
            }
        }
    }
    return QObject::eventFilter(obj, event);
}

void LayoutEventFilter::HandleWidgetClick(QWidget* widget) {
    int index = layout_->indexOf(widget);
    if (index != -1) {
        SetDefaultStyle();
        widget->setStyleSheet("border: 2px solid black; padding: 5px; background-color: gray;");
        emit cellSelected(index);
    }
}

void LayoutEventFilter::SetDefaultStyle() {
    for (int i = 0; i < layout_->count(); ++i) {
        QWidget* w = layout_->itemAt(i)->widget();
        if (w) {
            w->setStyleSheet("border: 1px solid gray; padding: 5px;");
        }
    }
}

ToolWidget::ToolWidget(QWidget* parent) : QWidget(parent) {
    setStyleSheet("border: 2px dashed #aaa;");

    QVBoxLayout* main_layout = new QVBoxLayout{this};
    main_layout->addWidget(new ComponentWidgets::Label{this});
    main_layout->addWidget(new QLabel{"Rectangle",this});
    main_layout->addWidget(new QLabel{"Ellipse", this});
    main_layout->addWidget(new QLabel{"Line", this});
    main_layout->addWidget(new QLabel{"Brush",this});
    main_layout->addSpacerItem(new QSpacerItem(0, 0,
                                QSizePolicy::Expanding, QSizePolicy::Expanding));

    setLayout(main_layout);

    event_filter_ = new LayoutEventFilter(main_layout, this);
    connect(event_filter_, &LayoutEventFilter::cellSelected,
            this, &ToolWidget::OnCellSelected);
}

void ToolWidget::OnCellSelected(int index) {
    comp_wgt_index_ = static_cast<ComponentWidgetIndex>(index);
    emit saveCurrentTool(comp_wgt_index_);
}

Canvas::Canvas(QWidget* parent) : QFrame(parent) {
    setAcceptDrops(true);
    setStyleSheet("background-color: white; border: 2px dashed #aaa;");
}

void Canvas::SetCurrentCell(ComponentWidgetIndex idx) {
    current_tool_ = idx;
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

ValueUpdatedWidgetsByObjName& Canvas::GetUpdatebleWidgets() {
    return value_updated_widgets_by_obj_name_;
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
}

void Canvas::paintEvent(QPaintEvent *event) {
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);


    for (const Shape& shape : shapes_) {
        painter.setPen(QPen(shape.color, 2));
        painter.setBrush(QBrush(shape.color, Qt::NoBrush));

        DrawShape(shape, painter);
    }

    if (drawing_) {
        painter.setPen(QPen(current_shape_.color, 2, Qt::DashLine));
        DrawShape(current_shape_, painter);
    }
}

void Canvas::DrawShape(const Shape& shape, QPainter& painter) {
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

ViewWidget::ViewWidget(QWidget *parent)
    : QWidget{parent} , canvas_{new Canvas{this}} {
    setWindowTitle("View Widget");
    setGeometry(100, 100, 800, 600);

    QHBoxLayout* layout = new QHBoxLayout{};
    setLayout(layout);

    tool_wgt_ = new ToolWidget{this};
    layout->addWidget(tool_wgt_,1);

    connect(tool_wgt_, &ToolWidget::saveCurrentTool,
            canvas_, &Canvas::SetCurrentCell);

    layout->addWidget(canvas_,4);
}

Canvas* ViewWidget::GetCanvas() {
    return canvas_;
}

}   //view_widget
