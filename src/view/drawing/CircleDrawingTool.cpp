#include "CircleDrawingTool.h"
#include "CirclePrimitive.h"
#include "LineStyleManager.h"

#include <QPainter>

void CircleDrawingTool::draw(QPainter& painter, BasePrimitive* primitive, bool isSelected) const
{
    auto* circle = static_cast<CirclePrimitive*>(primitive);
    if (!circle) return;

    // Делегируем отрисовку менеджеру стилей
    LineStyleManager::instance().drawEllipse(
        painter,
        QPointF(circle->getCenter().getX(), circle->getCenter().getY()),
        circle->getRadius(),
        circle->getRadius(), // Для окружности rx == ry
        circle->getLineType(),
        circle->getColor(),
        isSelected
        );
}
