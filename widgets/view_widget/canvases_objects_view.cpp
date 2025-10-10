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
    , figures_{new CanvasesItemsList{this}}
    , objects_{new CanvasesItemsList{this}} {
    QVBoxLayout* layout = new QVBoxLayout{};

    connect(canvas_, &Canvas::ItemAdded, this, [self = this] (ComponentWidgetIndex idx) {
        if(idx == ComponentWidgetIndex::None) {
            return;
        }
        if(idx == ComponentWidgetIndex::Label) {
            self->objects_->addItem(ComponentWidgetIndexToString(idx));
            return;
        }
        self->figures_->addItem(ComponentWidgetIndexToString(idx));
    });
    connect(figures_, &CanvasesItemsList::deleteItem,
                canvas, &Canvas::DeleteShapeByIndex);

    connect(objects_, &CanvasesItemsList::deleteItem,
            canvas, &Canvas::DeleteObjectByIndex);

    layout->addWidget(figures_);
    layout->addWidget(objects_);
    setLayout(layout);
}

}   //view_widget
