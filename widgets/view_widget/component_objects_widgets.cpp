#include "component_objects_widgets.h"

#include <QMouseEvent>
#include <QMenu>
#include <QVBoxLayout>

#include "label_settings_widget.h"

namespace ComponentObjectsWidgets {

QWidget* CurrentComponentByPtr(QWidget* component, QWidget* parent) {
    if (dynamic_cast<Label*>(component)) {
        return new Label{parent};
    } else {
        Q_ASSERT(false);
    }
}

Type ComponentObjectsStringToType(const QString& str_type) {
    if(str_type == "None")
        return Type::None;
    if(str_type == "Label")
        return Type::Label;
    if(str_type == "Other")
        return Type::Other;

    Q_ASSERT("ComponentObjectsWidgets: ComponentObjectsStringToType - unprocessed type");
    return {};
}

ComponentObjectWgtInterface* MakeComponentObjectsWgt(Type type, QWidget* parent) {
    if (type == Type::Label)
        return new Label(parent);

    Q_ASSERT("ComponentObjectsWidgets::MakeComponentObjectsWgt:  bad_type");
    return nullptr;
}

COW_ObserverBase::COW_ObserverBase(QObject* parent)
    : QObject{parent} {
}

void COW_ObserverBase::SetOnObjectNameChanged(OnOBjNameChanged on_object_name_changed) {
    on_object_name_changed_ = on_object_name_changed;
}

void COW_ObserverBase::CallOnObjectNameChanged(const QString& prev_name, const QString& name) {
    if(on_object_name_changed_)
        on_object_name_changed_(prev_name,name);
}

ComponentObjectWgtInterface::ComponentObjectWgtInterface(QWidget* parent)
    : QWidget{parent} {
}

Label::Label(QWidget* parent)
    : ComponentObjectWgtInterface(parent) {
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    setStyleSheet("background-color: #f0f0f0; border: 1px solid #999;");
    setAttribute(Qt::WA_DeleteOnClose);
    setMouseTracking(true);

    main_lbl_ = new QLabel("Label",this);
    QVBoxLayout* main_layout = new QVBoxLayout;
    main_layout->addWidget(main_lbl_);
    setLayout(main_layout);
}

void Label::SetObserver(COW_ObserverBase* observer) {
    observer_ = observer;
}

const QString Label::GetStringType() const {
    return "Label";
}

void Label::SetText(const QString& text) {
    main_lbl_->setText(text);
}

void Label::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        if (isInResizeArea(event->pos())) {
            resizing_ = true;
            drag_start_pos_ = event->globalPosition();
            start_geom_ = geometry();
            return;
        }
        QDrag* drag = new QDrag(this);
        QMimeData* mimeData = new QMimeData;
        mimeData->setText(QString::number((quintptr)this));
        drag->setMimeData(mimeData);
        drag->exec(Qt::MoveAction);
    }
    if (event->button() == Qt::RightButton) {
        showContextMenu(event->pos());
    }
}

void Label::mouseMoveEvent(QMouseEvent *event) {
    if (resizing_) {
        QPointF delta = event->globalPosition() - drag_start_pos_;
        QRect g = start_geom_;

        g.setRight(g.right() + delta.x());
        g.setBottom(g.bottom() + delta.y());

        g.setWidth(qMax(g.width(), minimumWidth()));
        g.setHeight(qMax(g.height(), minimumHeight()));

        setGeometry(g);
        return;
    }

    if (isInResizeArea(event->pos()))
        setCursor(Qt::SizeFDiagCursor);
    else
        unsetCursor();
}

void Label::mouseReleaseEvent(QMouseEvent *event) {
    Q_UNUSED(event);
    resizing_ = false;
}

void Label::mouseDoubleClickEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        LabelSettingsWidget label_stg_wgt{nullptr,main_lbl_->text(),this->objectName(),
                                          this->size()};
        label_stg_wgt.exec();
        LabelSettings label_stg = label_stg_wgt.GetSettings();
        main_lbl_->setText(label_stg.label);
        if(observer_)
            observer_->CallOnObjectNameChanged(this->objectName(), label_stg.object_name);
        setObjectName(label_stg.object_name);
        resize(label_stg.width,label_stg.height);
    }
}

void Label::showContextMenu(const QPoint& pos) {
    // QMenu menu(this);
    // QAction* action1 = menu.addAction("Delete");
    // connect(action1, &QAction::triggered, [self = this]() { self->close(); });
    // menu.exec(mapToGlobal(pos));
}

bool Label::isInResizeArea(const QPoint &p)
{
    int MARGIN = 8;
    return p.x() >= width()  - MARGIN &&
           p.y() >= height() - MARGIN;
}

}   //ComponentObjectsWidgets
