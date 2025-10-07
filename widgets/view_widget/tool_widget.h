#ifndef TOOL_WIDGET_H
#define TOOL_WIDGET_H

#include <QVBoxLayout>
#include <QWidget>

#include "domain.h"

namespace view_widget {

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

}   //view_widget

#endif // TOOL_WIDGET_H
