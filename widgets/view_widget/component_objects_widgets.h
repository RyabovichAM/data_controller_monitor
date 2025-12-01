#ifndef COMPONENT_WIDGETS_H
#define COMPONENT_WIDGETS_H

#include <QDrag>
#include <QLabel>
#include <QMimeData>

namespace ComponentObjectsWidgets {

enum class Type {
    None, Label, Other
};

QWidget* CurrentComponentByPtr(QWidget* component, QWidget* parent = nullptr);
Type ComponentObjectsStringToType(const QString& str_type);


class COW_ObserverBase  : QObject {
public:
    using OnOBjNameChanged = std::function<void(const QString& prev_obj_name, const QString& obj_name)>;

    explicit COW_ObserverBase(QObject* parent = nullptr);
    void SetOnObjectNameChanged(OnOBjNameChanged on_object_name_changed);
    void CallOnObjectNameChanged(const QString& prev_name, const QString& name);

private:
    OnOBjNameChanged on_object_name_changed_;
};

class ComponentObjectWgtInterface : public QWidget {
public:
    ComponentObjectWgtInterface(QWidget* parent=nullptr);
    virtual ~ComponentObjectWgtInterface() = default;

    virtual void SetObserver(COW_ObserverBase* observer) = 0;
    virtual const QString GetStringType() const = 0;
};

class Label : public ComponentObjectWgtInterface {
    Q_OBJECT
public:
    Label(QWidget* parent = nullptr);
    void SetObserver(COW_ObserverBase* observer) override;
    const QString GetStringType() const override;
    void SetText(const QString& text);

protected:
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;

private:
    COW_ObserverBase* observer_{nullptr};
    bool resizing_{false};
    QPointF drag_start_pos_;
    QRect start_geom_;
    QLabel* main_lbl_;
    void showContextMenu(const QPoint& pos);
    bool isInResizeArea(const QPoint &p);
};

ComponentObjectWgtInterface* MakeComponentObjectsWgt(Type type, QWidget* parent);

}   //ComponentObjectsWidgets

#endif // COMPONENT_WIDGETS_H
