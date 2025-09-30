#ifndef VIEW_WIDGET_H
#define VIEW_WIDGET_H

#include <QWidget>

#include <QtAssert>
#include <QEvent>
#include <QMouseEvent>
#include <QLabel>
#include <QVBoxLayout>
#include <QPainter>
#include <QPen>
#include <QPaintEvent>

#include "component_widgets.h"

namespace view_widget {

enum class ComponentWidgetIndex {
    Label, Rectangle, Ellipse, Line, Brush
};

class LayoutEventFilter : public QObject {
    Q_OBJECT
public:
    LayoutEventFilter(QVBoxLayout* layout, QObject* parent = nullptr);
protected:
    bool eventFilter(QObject* obj, QEvent* event) override;
signals:
    void cellSelected(int index);
private:
    QVBoxLayout* layout_;

    void HandleWidgetClick(QWidget* widget);
    void SetDefaultStyle();
};


class ToolWidget : public QWidget
{
    Q_OBJECT
public:
    ToolWidget(QWidget* parent = nullptr);
private slots:
    void OnCellSelected(int index);
private:
    LayoutEventFilter* event_filter_;
    ComponentWidgetIndex comp_wgt_index_;
signals:
    void saveCurrentTool(ComponentWidgetIndex idx);
};

using ValueUpdatedWidgetsByObjName = QHash<QString, QWidget*>;

class Canvas : public QFrame {
    Q_OBJECT
public:
    struct Shape {
        QColor color;
        ComponentWidgetIndex tool_type;
        QList<QPoint> points;
    };

    Canvas(QWidget* parent = nullptr);
    ValueUpdatedWidgetsByObjName& GetUpdatebleWidgets();

public slots:
    void SetCurrentCell(ComponentWidgetIndex idx);
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
    QList<Shape> shapes_;
    Shape current_shape_;
    bool drawing_;

    void DrawShape(const Shape& shape, QPainter& painter);
};

class ViewWidget : public QWidget {
    Q_OBJECT
public:
    explicit ViewWidget(QWidget *parent = nullptr);

    Canvas* GetCanvas();

private:
    ToolWidget* tool_wgt_;
    Canvas* canvas_;
};

}   //view_widget

#endif // VIEW_WIDGET_H
