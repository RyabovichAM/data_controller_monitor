#include "view_widget.h"

#include <QMimeData>

namespace view_widget {

ViewWidget::ViewWidget(QWidget *parent)
    : QWidget{parent}
    , tool_wgt_{new ToolWidget{this}}
    , canvas_{new Canvas{this}}
    , objects_viewer_{new CanvasesObjectsView(canvas_,this)} {
    setWindowTitle("View Widget");
    setGeometry(100, 100, 800, 600);

    QHBoxLayout* layout = new QHBoxLayout{};
    setLayout(layout);

    /*tool_wgt_ = new ToolWidget{this}*/;
    layout->addWidget(tool_wgt_,1);

    connect(tool_wgt_, &ToolWidget::saveCurrentTool,
            canvas_, &Canvas::SetCurrentComponentWidgetIndex);

    layout->addWidget(canvas_,4);
    layout->addWidget(objects_viewer_,1);
}

Canvas* ViewWidget::GetCanvas() {
    return canvas_;
}

}   //view_widget
