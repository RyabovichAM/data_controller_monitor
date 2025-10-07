#include "canvases_objects_view.h"

#include <QVBoxLayout>

namespace view_widget {

CanvasesFiguresList::CanvasesFiguresList(QWidget* parent)
    : QListWidget{parent} {
    setSelectionMode(QAbstractItemView::SingleSelection);
}

void CanvasesFiguresList::keyPressEvent(QKeyEvent *event) {
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
    , figures_{new CanvasesFiguresList{this}}
    , objects_{new QListWidget{this}} {
    QVBoxLayout* layout = new QVBoxLayout{};

    connect(canvas, &Canvas::FigureAdded, this, [self = this] (const QString&
                                                                  figure_type) {
        self->figures_->addItem(figure_type);
    });
    connect(figures_, &CanvasesFiguresList::deleteItem,
                canvas, &Canvas::DeleteShapeByIndex);

    layout->addWidget(figures_);
    layout->addWidget(objects_);
    setLayout(layout);
}

}   //view_widget
