#ifndef VIEW_WIDGET_H
#define VIEW_WIDGET_H

#include <QLineEdit>
#include <QWidget>

#include "tool_widget.h"
#include "canvas.h"
#include "canvases_objects_view.h"

namespace view_widget {

class ViewWidget : public QWidget {
    Q_OBJECT
public:
    explicit ViewWidget(QWidget *parent = nullptr);

    Canvas* GetCanvas();

private:
    ToolWidget* tool_wgt_;
    Canvas* canvas_;
    CanvasesObjectsView* objects_viewer_;
    QLineEdit* canvases_label_{nullptr};
};

}   //view_widget

#endif // VIEW_WIDGET_H
