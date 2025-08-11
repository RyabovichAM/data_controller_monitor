#include "component_widgets.h"

#include <QMouseEvent>
#include <QMenu>

#include "label_settings_widget.h"

namespace ComponentWidgets {

Label::Label(QWidget* parent)
    : QLabel("label",parent) {
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    setStyleSheet("background-color: #f0f0f0; border: 1px solid #999;");
    setAttribute(Qt::WA_DeleteOnClose);
}

void Label::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {

        QDrag* drag = new QDrag(this);
        QMimeData* mimeData = new QMimeData;

        // Pass a pointer to this widget (in text form)
        mimeData->setText(QString::number((quintptr)this));
        drag->setMimeData(mimeData);

        // Launch drag and drop (MoveAction - implies moving)
        drag->exec(Qt::MoveAction);
    }
    if (event->button() == Qt::RightButton) {
        showContextMenu(event->pos());
    }
}

void Label::mouseDoubleClickEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        LabelSettingsWidget label_stg_wgt{nullptr,this->text(),this->objectName(),
                                          this->size()};
        label_stg_wgt.exec();
        LabelSettings label_stg = label_stg_wgt.GetSettings();
        this->setText(label_stg.label);
        this->setObjectName(label_stg.object_name);
        this->setFixedSize(label_stg.width,label_stg.height);
    }
}

void Label::showContextMenu(const QPoint& pos) {
    QMenu menu(this);

    QAction* action1 = menu.addAction("Delete");

    connect(action1, &QAction::triggered, [self = this]() { self->close(); });

    menu.exec(mapToGlobal(pos));
}

QWidget* CurrentComponentByPtr(QWidget* component, QWidget* parent) {
    if (dynamic_cast<Label*>(component)) {
        return new Label{parent};
    } else {
        Q_ASSERT(false);
    }
}

}   //ComponentWidgets
