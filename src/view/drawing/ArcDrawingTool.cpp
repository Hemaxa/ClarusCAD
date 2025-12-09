#include "ArcDrawingTool.h"
#include "ArcPrimitive.h"
#include "LineStyleManager.h"
#include <QPainter>

void ArcDrawingTool::draw(QPainter& painter, BasePrimitive* primitive, bool isSelected) const {
    auto* arc = static_cast<ArcPrimitive*>(primitive);
    if (!arc) return;

    QRectF rect(arc->getCenter().getX() - arc->getRadius(),
                arc->getCenter().getY() - arc->getRadius(),
                arc->getRadius() * 2, arc->getRadius() * 2);

    // Qt drawArc принимает углы в 1/16 градуса
    int startAngle16 = static_cast<int>(arc->getStartAngle() * 16);
    int spanAngle16 = static_cast<int>(arc->getSpanAngle() * 16);

    QPen pen = LineStyleManager::instance().getPen(
        arc->getLineType(),
        arc->getColor(),
        false
        );

    if (isSelected) {
        QPen hPen = pen;
        hPen.setWidthF(hPen.widthF() + 8.0);
        QColor hColor = arc->getColor();
        hColor.setAlpha(100);
        hPen.setColor(hColor);
        painter.setPen(hPen);
        painter.setBrush(Qt::NoBrush);
        painter.drawArc(rect, startAngle16, spanAngle16);
    }

    painter.setPen(pen);
    painter.setBrush(Qt::NoBrush);
    painter.drawArc(rect, startAngle16, spanAngle16);
}
