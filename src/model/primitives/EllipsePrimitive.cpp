#include "EllipsePrimitive.h"
#include <algorithm>

EllipsePrimitive::EllipsePrimitive(const PointPrimitive& center, double rx, double ry, double rot)
    : m_center(center), m_radiusX(rx), m_radiusY(ry), m_rotation(rot) {}

QRectF EllipsePrimitive::getBoundingBox() const {
    // Упрощенный AABB (максимальный радиус)
    double maxR = std::max(m_radiusX, m_radiusY);
    return QRectF(m_center.getX() - maxR, m_center.getY() - maxR, maxR * 2, maxR * 2);
}
