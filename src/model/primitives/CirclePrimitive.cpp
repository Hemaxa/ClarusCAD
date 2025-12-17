#include "CirclePrimitive.h"
#include "LineStyleManager.h"
#include <cmath>

CirclePrimitive::CirclePrimitive(const PointPrimitive& center, double radius)
    : m_center(center), m_radius(radius) {}

const PointPrimitive& CirclePrimitive::getCenter() const { return m_center; }
void CirclePrimitive::setCenter(const PointPrimitive& center) { m_center = center; }

double CirclePrimitive::getRadius() const { return m_radius; }
void CirclePrimitive::setRadius(double radius) { m_radius = radius; }

double CirclePrimitive::getDiameter() const { return m_radius * 2.0; }
void CirclePrimitive::setDiameter(double diameter) { m_radius = diameter / 2.0; }

QRectF CirclePrimitive::getBoundingBox() const
{
    return QRectF(m_center.getX() - m_radius, m_center.getY() - m_radius, m_radius * 2.0, m_radius * 2.0);
}

void CirclePrimitive::draw(QPainter& painter, bool isSelected) const
{
    // Используем метод drawEllipse из менеджера, который умеет рисовать волны/зигзаги
    LineStyleManager::instance().drawEllipse(
        painter,
        QPointF(m_center.getX(), m_center.getY()),
        m_radius, m_radius,
        getLineType(),
        getColor(),
        isSelected
        );
}

bool CirclePrimitive::hitTest(const QPointF& point, double tolerance) const
{
    // Расстояние от клика до центра
    double dist = QLineF(point, QPointF(m_center.getX(), m_center.getY())).length();
    // Попадание, если расстояние отличается от радиуса не более чем на tolerance
    return std::abs(dist - m_radius) <= tolerance;
}

bool CirclePrimitive::intersects(const QRectF& rect) const
{
    // Упрощенно: пересечение с BoundingBox (для скорости)
    // Для идеальной точности (Green Selection) можно реализовать проверку круг-прямоугольник
    return rect.intersects(getBoundingBox());
}

bool CirclePrimitive::inside(const QRectF& rect) const
{
    return rect.contains(getBoundingBox());
}

QVector<QPointF> CirclePrimitive::getSnapPoints() const
{
    QVector<QPointF> points;
    double cx = m_center.getX();
    double cy = m_center.getY();

    points.append(QPointF(cx, cy)); // Центр

    // Квадранты (верх, низ, лево, право)
    points.append(QPointF(cx + m_radius, cy));
    points.append(QPointF(cx - m_radius, cy));
    points.append(QPointF(cx, cy + m_radius));
    points.append(QPointF(cx, cy - m_radius));

    return points;
}
