#include "ArcDrawingTool.h"
#include "ArcPrimitive.h"
#include "LineStyleManager.h"
#include <QPainter>

void ArcDrawingTool::draw(QPainter& painter, BasePrimitive* primitive, bool isSelected) const {
    auto* arc = static_cast<ArcPrimitive*>(primitive);
    if (!arc) return;

    QPointF center(arc->getCenter().getX(), arc->getCenter().getY());
    
    // Use LineStyleManager for proper wave/zigzag support
    LineStyleManager::instance().drawArc(
        painter,
        center,
        arc->getRadius(),
        arc->getStartAngle(),
        arc->getSpanAngle(),
        arc->getLineType(),
        arc->getColor(),
        isSelected
    );
}
