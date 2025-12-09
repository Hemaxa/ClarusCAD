#include "RectangleDrawingTool.h"
#include "RectanglePrimitive.h"
#include "LineStyleManager.h"
#include <QPainter>
#include <QPainterPath>

void RectangleDrawingTool::draw(QPainter& painter, BasePrimitive* primitive, bool isSelected) const {
    auto* rectPrim = static_cast<RectanglePrimitive*>(primitive);
    if (!rectPrim) return;

    QPainterPath path;
    double w = rectPrim->getWidth();
    double h = rectPrim->getHeight();
    double r = rectPrim->getCornerRadius();

    // Строим путь относительно центра (0,0)
    QRectF rBase(-w/2, -h/2, w, h);

    if (rectPrim->getCornerType() == CornerType::Fillet && r > 0) {
        path.addRoundedRect(rBase, r, r);
    }
    else if (rectPrim->getCornerType() == CornerType::Chamfer && r > 0) {
        // Построение фаски
        QPolygonF poly;
        poly << QPointF(-w/2 + r, -h/2) << QPointF(w/2 - r, -h/2)
             << QPointF(w/2, -h/2 + r) << QPointF(w/2, h/2 - r)
             << QPointF(w/2 - r, h/2) << QPointF(-w/2 + r, h/2)
             << QPointF(-w/2, h/2 - r) << QPointF(-w/2, -h/2 + r);
        poly.append(poly.first());
        path.addPolygon(poly);
    }
    else {
        path.addRect(rBase);
    }

    // Трансформация: Поворот -> Перенос
    QTransform t;
    t.translate(rectPrim->getCenter().getX(), rectPrim->getCenter().getY());
    t.rotate(rectPrim->getRotation());

    path = t.map(path);

    // Получаем перо через публичный метод LineStyleManager
    QPen pen = LineStyleManager::instance().getPen(
        rectPrim->getLineType(),
        rectPrim->getColor(),
        false // isSelected обрабатываем отдельно ниже
        );

    if (isSelected) {
        QPen hPen = pen;
        hPen.setWidthF(pen.widthF() + 8.0);
        QColor c = rectPrim->getColor();
        c.setAlpha(100);
        hPen.setColor(c);
        painter.setPen(hPen);
        painter.setBrush(Qt::NoBrush);
        painter.drawPath(path);
    }

    painter.setPen(pen);
    painter.setBrush(Qt::NoBrush);
    painter.drawPath(path);
}
