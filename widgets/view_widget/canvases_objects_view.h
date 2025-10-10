#ifndef CANVASES_OBJECTS_VIEW_H
#define CANVASES_OBJECTS_VIEW_H

#include <QKeyEvent>
#include <QListWidget>
#include <QWidget>

#include "canvas.h"

namespace view_widget {

class CanvasesItemsList : public QListWidget {
    Q_OBJECT
public:
    CanvasesItemsList(QWidget* parent = nullptr);

protected:
    void keyPressEvent(QKeyEvent *event) override;

signals:
    void deleteItem(int index);
};

class CanvasesObjectsView : public QWidget {
    Q_OBJECT
public:
    CanvasesObjectsView(Canvas* canvas, QWidget* parent = nullptr);

private:
    Canvas* canvas_;
    CanvasesItemsList* figures_;
    CanvasesItemsList* objects_;
};

}   //view_widget

#endif // CANVASES_OBJECTS_VIEW_H
