#include "tool_widget.h"

#include <QEvent>
#include <QMouseEvent>

#include "component_widgets.h"

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

}   //view_widget
