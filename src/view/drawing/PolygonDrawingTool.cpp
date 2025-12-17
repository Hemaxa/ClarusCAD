#include "PolygonDrawingTool.h"
#include "PolygonPrimitive.h"
#include "LineStyleManager.h"

#include <QPainter>

void PolygonDrawingTool::draw(QPainter& painter, BasePrimitive* primitive, bool isSelected) const
{
    auto* polygon = static_cast<PolygonPrimitive*>(primitive);
    if (!polygon) return;
    
    // Делегируем отрисовку самому примитиву (он уже умеет рисовать через LineStyleManager)
    polygon->draw(painter, isSelected);
}
