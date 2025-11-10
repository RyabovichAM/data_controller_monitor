#include "view_widget.h"


namespace view_widget {

ViewWidget::ViewWidget(QWidget *parent)
    : QWidget{parent}
    , tool_wgt_{new ToolWidget{this}}
    , canvas_{new Canvas{this}}
    , objects_viewer_{new CanvasesObjectsView(canvas_,this)}
    , canvases_label_{new QLineEdit{this}} {
    setWindowTitle("View Widget");

    QHBoxLayout* layout = new QHBoxLayout{};
    setLayout(layout);

    layout->addWidget(tool_wgt_,1);

    connect(tool_wgt_, &ToolWidget::saveCurrentTool,
            canvas_, &Canvas::SetCurrentComponentWidgetType);

    QVBoxLayout* canvases_layout = new QVBoxLayout{};
    canvases_label_->setPlaceholderText("Canvases label");
    canvases_layout->addWidget(canvases_label_);
    canvases_layout->addWidget(canvas_);
    layout->addLayout(canvases_layout,4);
    layout->addWidget(objects_viewer_,1);
}

Canvas* ViewWidget::GetCanvas() {
    canvas_->SetLabel(canvases_label_->text());
    return canvas_;
}

}   //view_widget
