#include "view_widget.h"


namespace view_widget {

ViewWidget::ViewWidget(QWidget *parent, bool new_view_widget)
    : QWidget{parent}
    , tool_wgt_{new ToolWidget{this}}
    , canvases_label_{new QLineEdit{this}} {
    setWindowTitle("View Widget");

    if(new_view_widget) {
        canvas_ = new Canvas{this};
        objects_viewer_ = new CanvasesObjectsView(canvas_,this);
    }

    main_layout_ = new QHBoxLayout{};
    setLayout(main_layout_);

    main_layout_->addWidget(tool_wgt_,1);

    connect(tool_wgt_, &ToolWidget::saveCurrentTool,
            canvas_, &Canvas::SetCurrentComponentWidgetType);

    canvases_layout_ = new QVBoxLayout{};
    canvases_label_->setPlaceholderText("Canvases label");
    canvases_layout_->addWidget(canvases_label_);
    canvases_layout_->addWidget(canvas_);
    main_layout_->addLayout(canvases_layout_,4);
    main_layout_->addWidget(objects_viewer_,1);
}

void ViewWidget::SetUp(Canvas* canvas) {
    canvas_->setParent(nullptr);
    disconnect(tool_wgt_, &ToolWidget::saveCurrentTool,
            canvas_, &Canvas::SetCurrentComponentWidgetType);
    canvases_layout_->removeWidget(canvas_);
    delete canvas_;
    canvas_ = canvas;
    for(auto component_wgt : canvas_->findChildren<QWidget*>()) {
        component_wgt->setAttribute(Qt::WA_TransparentForMouseEvents,false);
    }
    canvas_->setAttribute(Qt::WA_TransparentForMouseEvents, false);
    canvases_layout_->addWidget(canvas_);
    if(!objects_viewer_) {
        objects_viewer_ = new CanvasesObjectsView(canvas_,this);
    } else {
        delete objects_viewer_;
        objects_viewer_ = new CanvasesObjectsView(canvas_,this);
    }
    main_layout_->addWidget(objects_viewer_,1);
    connect(tool_wgt_, &ToolWidget::saveCurrentTool,
            canvas_, &Canvas::SetCurrentComponentWidgetType);

    for( auto& shape : canvas_->GetShapes()) {
        objects_viewer_->AddShape(shape);
    }

    for(auto& object : canvas_->GetObjects()) {
        objects_viewer_->AddObject(object->GetStringType());
    }
}

Canvas* ViewWidget::GetCanvas() {
    canvas_->SetLabel(canvases_label_->text());
    return canvas_;
}

}   //view_widget
