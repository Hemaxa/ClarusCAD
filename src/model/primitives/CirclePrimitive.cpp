#include "CirclePrimitive.h"

CirclePrimitive::CirclePrimitive(const PointPrimitive& center, double radius)
    : m_center(center), m_radius(radius)
{
}

const PointPrimitive& CirclePrimitive::getCenter() const
{
    return m_center;
}

void CirclePrimitive::setCenter(const PointPrimitive& center)
{
    m_center = center;
}

double CirclePrimitive::getRadius() const
{
    return m_radius;
}

void CirclePrimitive::setRadius(double radius)
{
    m_radius = radius;
}

double CirclePrimitive::getDiameter() const
{
    return m_radius * 2.0;
}

void CirclePrimitive::setDiameter(double diameter)
{
    m_radius = diameter / 2.0;
}

QRectF CirclePrimitive::getBoundingBox() const
{
    //возвращает квадрат, описывающий окружность
    return QRectF(m_center.getX() - m_radius, m_center.getY() - m_radius, m_radius * 2.0, m_radius * 2.0);
}
