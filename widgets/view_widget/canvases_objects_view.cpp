#include "canvases_objects_view.h"

#include <QVBoxLayout>

namespace view_widget {

CanvasesItemsList::CanvasesItemsList(QWidget* parent)
    : QListWidget{parent} {
    setSelectionMode(QAbstractItemView::SingleSelection);
}

void CanvasesItemsList::keyPressEvent(QKeyEvent *event) {
    if (event->key() == Qt::Key_Delete) {
        QListWidgetItem *currentItem = this->currentItem();

        if (currentItem) {
            int currentRow = this->currentRow();
            delete takeItem(currentRow);
            deleteItem(currentRow);

            if (count() > 0) {
                if (currentRow >= count())
                    setCurrentRow(count() - 1);
                else
                    setCurrentRow(currentRow);
            }
        }
    }
    else {
        QListWidget::keyPressEvent(event);
    }
}

CanvasesObjectsView::CanvasesObjectsView(Canvas* canvas, QWidget* parent)
    : canvas_{canvas}
    , QWidget{parent}
    , shapes_{new CanvasesItemsList{this}}
    , objects_{new CanvasesItemsList{this}} {
    QVBoxLayout* layout = new QVBoxLayout{};

    connect(canvas_, &Canvas::ItemAdded, this, [self = this]
            (ComponentWidgetType idx, const QString& comp_wgt_sub_type) {
        if(idx == ComponentWidgetType::None || idx == ComponentWidgetType::Other) {
            return;
        }
        if(idx == ComponentWidgetType::Oblect) {
            self->objects_->addItem(comp_wgt_sub_type);
            return;
        }
        if(idx == ComponentWidgetType::Shape) {
            self->shapes_->addItem(comp_wgt_sub_type);
            return;
        }
    });
    connect(shapes_, &CanvasesItemsList::deleteItem,
                canvas, &Canvas::DeleteShapeByIndex);

    connect(objects_, &CanvasesItemsList::deleteItem,
            canvas, &Canvas::DeleteObjectByIndex);

    layout->addWidget(shapes_);
    layout->addWidget(objects_);
    setLayout(layout);
}

void CanvasesObjectsView::AddShape(const Shape& shape) {
    shapes_->addItem(Shape::TypeToString(shape.tool_type));
}

void CanvasesObjectsView::AddObject(const QString& object_name) {
    objects_->addItem(object_name);
}

void CanvasesObjectsView::SetCanvas(Canvas* canvas) {
    canvas_ = canvas;
}

}   //view_widget
