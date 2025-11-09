#include "domain.h"

namespace view_widget {

QString ComponentWidgetShapeToString(Shape::Type idx) {
    switch (idx) {
    case Shape::Type::None:
        return "None";
    case Shape::Type::Rectangle:
        return "Rectangle";
    case Shape::Type::Ellipse:
        return "Ellipse";
    case Shape::Type::Line:
        return "Line";
    case Shape::Type::Brush:
        return "Brush";
    case Shape::Type::Other:
        return "Other";
    default:
        Q_ASSERT("View_widget: domain: ComponentWidgetShapeToString - unprocessed type");
    }
    return {};
}

Shape::Type ComponentWidgetStringToShapeType(const QString& str_type) {
    if(str_type == "None")
        return Shape::Type::None;
    if(str_type == "Rectangle")
        return Shape::Type::Rectangle;
    if(str_type == "Ellipse")
        return Shape::Type::Ellipse;
    if(str_type == "Line")
        return Shape::Type::Line;
    if(str_type == "Brush")
        return Shape::Type::Brush;
    if(str_type == "Other")
        return Shape::Type::Other;

    Q_ASSERT("View_widget: domain: ComponentWidgetStringToShapeType - unprocessed type");
    return {};
}

QString Shape::TypeToString(Type type) {
    return ComponentWidgetShapeToString(type);
}

Shape::Type Shape::StringToType(const QString& str_type) {
    return ComponentWidgetStringToShapeType(str_type);
}

}   //view_widget
