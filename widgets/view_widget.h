#ifndef VIEW_WIDGET_H
#define VIEW_WIDGET_H

#include <QFrame>
#include <QWidget>

class DragArea : public QFrame {
    Q_OBJECT
public:
    DragArea(QWidget* parent = nullptr);
};


class DropArea : public QFrame {
    Q_OBJECT
public:
    DropArea(QWidget* parent = nullptr);

protected:
    void dragEnterEvent(QDragEnterEvent* event) override;
    void dropEvent(QDropEvent* event) override;

private:
};


class ViewWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ViewWidget(QWidget *parent = nullptr);

private:
    DropArea* dropArea;
    DragArea* dragArea;
};


#endif // VIEW_WIDGET_H
