#include "Point.h"

Point::Point(double x, double y) : m_x(x), m_y(y)
{
}

double Point::x() const
{
    return m_x;
}

void Point::setX(double x)
{
    m_x = x;
}

double Point::y() const
{
    return m_y;
}

void Point::setY(double y)
{
    m_y = y;
}
