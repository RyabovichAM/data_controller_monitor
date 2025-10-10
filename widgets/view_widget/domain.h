#ifndef DOMAIN_H
#define DOMAIN_H

#include <QString>

namespace view_widget {

enum class ComponentWidgetIndex {
    None, Label, Rectangle, Ellipse, Line, Brush
};

QString ComponentWidgetIndexToString(ComponentWidgetIndex idx);

}   //view_widget

#endif // DOMAIN_H
