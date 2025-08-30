#ifndef VIEW_WIDGET_H
#define VIEW_WIDGET_H

#include <QFrame>
#include <QWidget>

class DragArea : public QFrame {
    Q_OBJECT
public:
    DragArea(QWidget* parent = nullptr);
};


using ValueUpdatedWidgetsByObjName = QHash<QString, QWidget*>;
class DropArea : public QFrame {
    Q_OBJECT

public:
    DropArea(QWidget* parent = nullptr);
    ValueUpdatedWidgetsByObjName& GetUpdatebleWidgets();

protected:
    void dragEnterEvent(QDragEnterEvent* event) override;
    void dropEvent(QDropEvent* event) override;

private:
    ValueUpdatedWidgetsByObjName value_updated_widgets_by_obj_name_;
};


class ViewWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ViewWidget(QWidget *parent = nullptr);

    DropArea* GetDropArea();

private:
    DropArea* dropArea_;
    DragArea* dragArea_;
};


#endif // VIEW_WIDGET_H
