#ifndef DOMAIN_H
#define DOMAIN_H

#include <QColor>
#include <QString>
#include <QPoint>

namespace view_widget {

struct Shape {
    enum class Type {
        None, Rectangle, Ellipse, Line, Brush, Other
    };

    QColor color;
    Type tool_type;
    QList<QPoint> points;

    static QString TypeToString(Type type);
    static Type StringToType(const QString& str_type);
};

enum class ComponentWidgetType {
    None, Oblect, Shape, Other
};

QString ComponentWidgetShapeToString(Shape::Type idx);
Shape::Type ComponentWidgetStringToShapeType(const QString& str_type);

}   //view_widget

#endif // DOMAIN_H
