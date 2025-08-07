#ifndef COMPONENT_WIDGETS_H
#define COMPONENT_WIDGETS_H

#include <QDrag>
#include <QLabel>
#include <QMimeData>

namespace ComponentWidgets {

class Label : public QLabel {
    Q_OBJECT
public:
    Label(QWidget* parent = nullptr);

protected:
    void mousePressEvent(QMouseEvent* event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;

private:
    void showContextMenu(const QPoint& pos);
};

QWidget* CurrentComponentByPtr(QWidget* component, QWidget* parent);

}   //ComponentWidgets

#endif // COMPONENT_WIDGETS_H
