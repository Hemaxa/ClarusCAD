#include "DimensionDrawingTool.h"
#include "BasePrimitive.h"

void DimensionDrawingTool::draw(QPainter& painter, BasePrimitive* primitive, bool isSelected) const
{
    if (!primitive) return;
    primitive->draw(painter, isSelected);
}
