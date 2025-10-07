#ifndef CANVASES_OBJECTS_VIEW_H
#define CANVASES_OBJECTS_VIEW_H

#include <QKeyEvent>
#include <QListWidget>
#include <QWidget>

#include "canvas.h"

namespace view_widget {

class CanvasesFiguresList : public QListWidget {
    Q_OBJECT
public:
    CanvasesFiguresList(QWidget* parent = nullptr);

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
    CanvasesFiguresList* figures_;
    QListWidget* objects_;
};

}   //view_widget

#endif // CANVASES_OBJECTS_VIEW_H
