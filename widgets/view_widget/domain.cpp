#include "domain.h"

namespace view_widget {

QString ComponentWidgetIndexToString(ComponentWidgetIndex idx) {
    switch (idx) {
    case ComponentWidgetIndex::Label:
        return "Label";
    case ComponentWidgetIndex::Rectangle:
        return "Rectangle";
    case ComponentWidgetIndex::Ellipse:
        return "Ellipse";
    case ComponentWidgetIndex::Line:
        return "Line";
    case ComponentWidgetIndex::Brush:
        return "Brush";
    default:
        Q_ASSERT("View_widget: ComponentWidgetIndexToString - unprocessed type");
    }
    return {};
}

}   //view_widget
