#include "ArcPrimitive.h"
#include "LineStyleManager.h"

#include <cmath>

namespace {
QPointF pointOnArc(const QPointF& center, double radius, double angleDeg)
{
    const double angleRad = angleDeg * M_PI / 180.0;
    return QPointF(center.x() + radius * std::cos(angleRad),
                   center.y() - radius * std::sin(angleRad));
}
}

ArcPrimitive::ArcPrimitive(const PointPrimitive& center, double radius, double startAngle, double spanAngle)
    : m_center(center), m_radius(radius), m_startAngle(startAngle), m_spanAngle(spanAngle) {}

QRectF ArcPrimitive::getBoundingBox() const {
    return QRectF(m_center.getX() - m_radius, m_center.getY() - m_radius, m_radius * 2, m_radius * 2);
}

void ArcPrimitive::draw(QPainter& painter, bool isSelected) const {
    QRectF rect(m_center.getX() - m_radius,
                m_center.getY() - m_radius,
                m_radius * 2, m_radius * 2);

    int startAngle16 = static_cast<int>(m_startAngle * 16);
    int spanAngle16 = static_cast<int>(m_spanAngle * 16);

    QPen pen = LineStyleManager::instance().getPen(getLineType(), getColor(), false);

    if (isSelected) {
        QPen hPen = pen;
        hPen.setWidthF(pen.widthF() + 8.0);
        QColor c = getColor(); c.setAlpha(100);
        hPen.setColor(c);
        painter.setPen(hPen);
        painter.setBrush(Qt::NoBrush);
        painter.drawArc(rect, startAngle16, spanAngle16);
    }

    painter.setPen(pen);
    painter.setBrush(Qt::NoBrush);
    painter.drawArc(rect, startAngle16, spanAngle16);
}

bool ArcPrimitive::hitTest(const QPointF& point, double tolerance) const {
    // 1. Проверяем расстояние до центра
    double dist = QLineF(point, QPointF(m_center.getX(), m_center.getY())).length();
    if (std::abs(dist - m_radius) > tolerance) return false;

    // 2. Проверяем угол
    QLineF line(QPointF(m_center.getX(), m_center.getY()), point);
    double angle = line.angle(); // 0..360 (counter-clockwise from 3 o'clock in Qt coords inverted?)

    // Qt QLineF::angle(): Returns angle in degrees, 0..360.
    // 0 is 3 o'clock, increases counter-clockwise (if Y axis points down, but we flipped it).
    // Let's assume standard math.

    // Нормализуем углы в 0..360
    double start = m_startAngle;
    while(start < 0) start += 360.0;
    while(start >= 360) start -= 360.0;

    double end = start + m_spanAngle;

    // Проверка попадания angle в интервал [start, end] с учетом перехода через 0
    // Упрощенно:
    double relAngle = angle - start;
    while (relAngle < 0) relAngle += 360.0;

    if (m_spanAngle >= 0) {
        return relAngle <= m_spanAngle;
    } else {
        // Если span отрицательный (рисовали в другую сторону)
        // relAngle должен быть "большим" (близким к 360) или span тоже приводим
        return relAngle >= (360.0 + m_spanAngle);
    }
}

bool ArcPrimitive::intersects(const QRectF& rect) const {
    return rect.intersects(getBoundingBox());
}

bool ArcPrimitive::inside(const QRectF& rect) const {
    return rect.contains(getBoundingBox());
}

QVector<QPointF> ArcPrimitive::getSnapPoints() const {
    QVector<QPointF> points;
    const QPointF center(m_center.getX(), m_center.getY());
    points.append(center); // Центр
    points.append(pointOnArc(center, m_radius, m_startAngle));
    points.append(pointOnArc(center, m_radius, m_startAngle + m_spanAngle));
    points.append(pointOnArc(center, m_radius, m_startAngle + m_spanAngle / 2.0));

    return points;
}
