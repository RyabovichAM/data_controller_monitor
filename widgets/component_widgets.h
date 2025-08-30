#ifndef COMPONENT_WIDGETS_H
#define COMPONENT_WIDGETS_H

#include <QDrag>
#include <QLabel>
#include <QMimeData>

namespace ComponentWidgets {

class CW_ObserverBase  : QObject {
public:
    using OnOBjNameChanged = std::function<void(const QString& prev_obj_name, const QString& obj_name)>;

    explicit CW_ObserverBase(QObject* parent = nullptr);
    void SetOnObjectNameChanged(OnOBjNameChanged on_object_name_changed);
    void CallOnObjectNameChanged(const QString& prev_name, const QString& name);

private:
    OnOBjNameChanged on_object_name_changed_;
};

class Label : public QLabel {
    Q_OBJECT
public:
    Label(QWidget* parent = nullptr);
    void SetObserver(CW_ObserverBase* observer);

protected:
    void mousePressEvent(QMouseEvent* event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;

private:
    CW_ObserverBase* observer_;
    void showContextMenu(const QPoint& pos);
};

QWidget* CurrentComponentByPtr(QWidget* component, QWidget* parent);

}   //ComponentWidgets

#endif // COMPONENT_WIDGETS_H
