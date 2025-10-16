#ifndef CANVAS_H
#define CANVAS_H

#include <QFrame>

#include "domain.h"

namespace view_widget {

struct Shape {
    QColor color;
    ComponentWidgetIndex tool_type;
    QList<QPoint> points;

    QString TypeToString() const;
};

using ValueUpdatedWidgetsByObjName = QHash<QString, QWidget*>;

class Canvas : public QFrame {
    Q_OBJECT
public:

    Canvas(QWidget* parent = nullptr);
    ValueUpdatedWidgetsByObjName& GetUpdatebleWidgets();
    QList<view_widget::Shape>& GetShapes();
    void SetLabel(const QString& label);
    QString GetLabel() const;

public slots:
    void SetCurrentComponentWidgetIndex(ComponentWidgetIndex idx);
    void DeleteShapeByIndex(int index);
    void DeleteObjectByIndex(int index);

protected:
    void dragEnterEvent(QDragEnterEvent* event) override;
    void dropEvent(QDropEvent* event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void paintEvent(QPaintEvent *event) override;

private:
    ValueUpdatedWidgetsByObjName value_updated_widgets_by_obj_name_;
    ComponentWidgetIndex current_tool_;
    QList<view_widget::Shape> shapes_;
    view_widget::Shape current_shape_;
    bool drawing_;
    QString label_;

    void DrawShape(const view_widget::Shape& shape, QPainter& painter);

signals:
    void ItemAdded(ComponentWidgetIndex idx);
};

}   //view_widget

#endif // CANVAS_H
