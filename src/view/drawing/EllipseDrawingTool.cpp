#include "EllipseDrawingTool.h"
#include "EllipsePrimitive.h"
#include "LineStyleManager.h"
#include <QPainter>

void EllipseDrawingTool::draw(QPainter& painter, BasePrimitive* primitive, bool isSelected) const {
    auto* ell = static_cast<EllipsePrimitive*>(primitive);
    if (!ell) return;

    painter.save();
    // Трансформация для поворота
    painter.translate(ell->getCenter().getX(), ell->getCenter().getY());
    painter.rotate(ell->getRotation()); // если есть поворот

    // Рисуем в (0,0)
    LineStyleManager::instance().drawEllipse(
        painter,
        QPointF(0, 0), // Центр уже перенесен
        ell->getRadiusX(),
        ell->getRadiusY(),
        ell->getLineType(),
        ell->getColor(),
        isSelected
        );

    painter.restore();
}
