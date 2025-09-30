#include "PointPrimitive.h"

#define _USE_MATH_DEFINES
#include <cmath>

AngleUnit PointPrimitive::s_angleUnit = AngleUnit::Degrees;

PointPrimitive::PointPrimitive(double x, double y) : m_x(x), m_y(y) {}

void PointPrimitive::setAngleUnit(AngleUnit unit)
{
    s_angleUnit = unit;
}

AngleUnit PointPrimitive::getAngleUnit()
{
    return s_angleUnit;
}

double PointPrimitive::getX() const
{
    return m_x;
}

void PointPrimitive::setX(double x)
{
    m_x = x;
}

double PointPrimitive::getY() const
{
    return m_y;
}

void PointPrimitive::setY(double y)
{
    m_y = y;
}

double PointPrimitive::getRadius() const
{
    //теорема Пифагора
    return std::sqrt(m_x * m_x + m_y * m_y);
}

double PointPrimitive::getAngle() const
{
    //получение угла в радианах
    double angleRad = std::atan2(m_y, m_x);

    //если нужны градусы, то выполняется перевод
    if (s_angleUnit == AngleUnit::Degrees) {
        return angleRad * 180.0 / M_PI;
    }

    return angleRad;
}

void PointPrimitive::setPolar(double radius, double angle)
{
    double angleRad = angle;

    //если пришли градусы, то конвертирует
    if (s_angleUnit == AngleUnit::Degrees) {
        angleRad = angle * M_PI / 180.0;
    }

    m_x = radius * std::cos(angleRad);
    m_y = radius * std::sin(angleRad);
}
