#include "SplineDrawingTool.h"
#include "SplinePrimitive.h"

#include <QPainter>

void SplineDrawingTool::draw(QPainter& painter, BasePrimitive* primitive, bool isSelected) const
{
    auto* spline = static_cast<SplinePrimitive*>(primitive);
    if (!spline) return;
    
    spline->draw(painter, isSelected);
}
