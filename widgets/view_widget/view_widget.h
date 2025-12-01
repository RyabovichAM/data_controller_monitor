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
    explicit ViewWidget(QWidget *parent = nullptr, bool new_view_widget = true);

    Canvas* GetCanvas();
    void SetUp(Canvas* canvas);

private:
    ToolWidget* tool_wgt_{nullptr};
    Canvas* canvas_{nullptr};
    CanvasesObjectsView* objects_viewer_{nullptr};
    QLineEdit* canvases_label_{nullptr};
    QHBoxLayout* main_layout_;
    QVBoxLayout* canvases_layout_;
};

}   //view_widget

#endif // VIEW_WIDGET_H
