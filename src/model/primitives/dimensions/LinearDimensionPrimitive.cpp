// LinearDimensionPrimitive.cpp
#include "LinearDimensionPrimitive.h"
#include <cmath>
#include <algorithm>

void LinearDimensionPrimitive::recalculateValue() {
    double dx = m_endPoint.x() - m_startPoint.x();
    double dy = m_endPoint.y() - m_startPoint.y();
    m_measuredValue = std::sqrt(dx * dx + dy * dy);
}

void LinearDimensionPrimitive::draw(QPainter& painter, bool isSelected) const {
    painter.save();

    double currentScale = std::abs(painter.transform().m11());
    if (currentScale < 0.0001) currentScale = 1.0;
    
    double textHeight = m_style.textHeight / currentScale;
    double arrowSize = m_style.arrowSize / currentScale;
    double extensionLineOffset = m_style.extensionLineOffset / currentScale;
    double extensionLineExtend = m_style.extensionLineExtend / currentScale;
    
    double dx = m_endPoint.x() - m_startPoint.x();
    double dy = m_endPoint.y() - m_startPoint.y();
    double angle = std::atan2(dy, dx);
    double length = m_measuredValue;
    if (length < 0.0001) {
        painter.restore();
        return;
    }
    
    // Normal vector
    double nx = -dy / length;
    double ny = dx / length;
    
    // Calculate dimension line distance from startPoint along the normal
    double vx = m_dimensionLinePos.x() - m_startPoint.x();
    double vy = m_dimensionLinePos.y() - m_startPoint.y();
    double dist = vx * nx + vy * ny; 
    
    // Start and End points of the dimension line
    QPointF dimStart(m_startPoint.x() + nx * dist, m_startPoint.y() + ny * dist);
    QPointF dimEnd(m_endPoint.x() + nx * dist, m_endPoint.y() + ny * dist);
    
    // Direction of extension lines (from startPoint to dimStart)
    double extLen = std::abs(dist);
    double dirExtX = (extLen > 0) ? (nx * dist / extLen) : nx;
    double dirExtY = (extLen > 0) ? (ny * dist / extLen) : ny;
    
    QPointF extStart1, extStart2;
    if (extLen > extensionLineOffset) {
        extStart1 = QPointF(m_startPoint.x() + dirExtX * extensionLineOffset, m_startPoint.y() + dirExtY * extensionLineOffset);
        extStart2 = QPointF(m_endPoint.x() + dirExtX * extensionLineOffset, m_endPoint.y() + dirExtY * extensionLineOffset);
    } else {
        extStart1 = m_startPoint;
        extStart2 = m_endPoint;
    }
    
    QPointF extEnd1(dimStart.x() + dirExtX * extensionLineExtend, dimStart.y() + dirExtY * extensionLineExtend);
    QPointF extEnd2(dimEnd.x() + dirExtX * extensionLineExtend, dimEnd.y() + dirExtY * extensionLineExtend);
    
    QPen pen(m_style.dimensionLineColor);
    pen.setWidth(0); // Cosmetic pen
    if (isSelected) {
        pen.setColor(Qt::red);
    }
    painter.setPen(pen);
    
    // Draw extension lines
    painter.drawLine(extStart1, extEnd1);
    painter.drawLine(extStart2, extEnd2);
    
    // Draw dimension line
    painter.drawLine(dimStart, dimEnd);
    
    // Draw arrows
    painter.setBrush(pen.color());
    
    double arrowAngle1 = angle;
    QPolygonF arrow1;
    arrow1 << dimStart;
    arrow1 << QPointF(dimStart.x() + arrowSize * std::cos(arrowAngle1 + 0.26), dimStart.y() + arrowSize * std::sin(arrowAngle1 + 0.26));
    arrow1 << QPointF(dimStart.x() + arrowSize * std::cos(arrowAngle1 - 0.26), dimStart.y() + arrowSize * std::sin(arrowAngle1 - 0.26));
    painter.drawPolygon(arrow1);
    
    double arrowAngle2 = angle + M_PI;
    QPolygonF arrow2;
    arrow2 << dimEnd;
    arrow2 << QPointF(dimEnd.x() + arrowSize * std::cos(arrowAngle2 + 0.26), dimEnd.y() + arrowSize * std::sin(arrowAngle2 + 0.26));
    arrow2 << QPointF(dimEnd.x() + arrowSize * std::cos(arrowAngle2 - 0.26), dimEnd.y() + arrowSize * std::sin(arrowAngle2 - 0.26));
    painter.drawPolygon(arrow2);
    
    // Draw text
    QString text = m_customText.isEmpty() ? QString::number(m_measuredValue, 'f', 2) : m_customText;
    
    QFont font = painter.font();
    font.setPointSizeF(textHeight);
    painter.setFont(font);
    
    QPointF midPoint((dimStart.x() + dimEnd.x()) / 2.0, (dimStart.y() + dimEnd.y()) / 2.0);
    
    double textAngle = angle * 180.0 / M_PI;
    
    // Keep text upright
    if (textAngle > 90.0) textAngle -= 180.0;
    else if (textAngle < -90.0) textAngle += 180.0;
    
    painter.translate(midPoint);
    painter.rotate(textAngle);
    
    QFontMetricsF fm(font);
    double textWidth = fm.horizontalAdvance(text);
    double textH = fm.height();
    double textOffset = 1.0 / currentScale;
    
    painter.drawText(QRectF(-textWidth / 2.0, -textH - textOffset, textWidth, textH), Qt::AlignCenter, text);
    
    painter.restore();
}

QRectF LinearDimensionPrimitive::getBoundingBox() const {
    // Упрощенный вариант, охватывающий 3 основные точки (start, end, pos)
    double minX = std::min({m_startPoint.x(), m_endPoint.x(), m_dimensionLinePos.x()});
    double maxX = std::max({m_startPoint.x(), m_endPoint.x(), m_dimensionLinePos.x()});
    double minY = std::min({m_startPoint.y(), m_endPoint.y(), m_dimensionLinePos.y()});
    double maxY = std::max({m_startPoint.y(), m_endPoint.y(), m_dimensionLinePos.y()});

    QRectF bbox(QPointF(minX, minY), QPointF(maxX, maxY));

    bbox.adjust(-25.0, -25.0, 25.0, 25.0);
    
    return bbox;
}

bool LinearDimensionPrimitive::hitTest(const QPointF& point, double tolerance) const {
    // Упрощенный вариант: проверка попадания в расширенный bounding box
    QRectF bbox = getBoundingBox();
    bbox.adjust(-tolerance, -tolerance, tolerance, tolerance);
    return bbox.contains(point);
}

bool LinearDimensionPrimitive::intersects(const QRectF& rect) const {
    return getBoundingBox().intersects(rect);
}

bool LinearDimensionPrimitive::inside(const QRectF& rect) const {
    return rect.contains(getBoundingBox());
}

QVector<QPointF> LinearDimensionPrimitive::getSnapPoints() const {
    return {m_startPoint, m_endPoint, m_dimensionLinePos};
}
