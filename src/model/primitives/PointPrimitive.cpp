#include "PointPrimitive.h"

PointPrimitive::PointPrimitive(double x, double y) : m_x(x), m_y(y) {}

double PointPrimitive::getX() const
{
    return m_x;
}

void PointPrimitive::setX(double x) {
    m_x = x;
}

double PointPrimitive::getY() const {
    return m_y;
}

void PointPrimitive::setY(double y) {
    m_y = y;
}
