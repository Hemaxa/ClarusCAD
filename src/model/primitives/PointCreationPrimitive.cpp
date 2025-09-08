#include "PointCreationPrimitive.h"

PointCreationPrimitive::PointCreationPrimitive(double x, double y) : m_x(x), m_y(y) {
}

double PointCreationPrimitive::x() const
{
    return m_x;
}

void PointCreationPrimitive::setX(double x) {
    m_x = x;
}

double PointCreationPrimitive::y() const {
    return m_y;
}

void PointCreationPrimitive::setY(double y) {
    m_y = y;
}
