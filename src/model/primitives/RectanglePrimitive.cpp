#include "RectanglePrimitive.h"
#include <QtMath>

RectanglePrimitive::RectanglePrimitive(const PointPrimitive& center, double width, double height, double rotation)
    : m_center(center), m_width(width), m_height(height), m_rotation(rotation) {}

QRectF RectanglePrimitive::getBoundingBox() const {
    // Вычисляем AABB для повернутого прямоугольника
    // Берем половину диагонали как радиус описанной окружности для простоты
    double diag = std::sqrt(m_width*m_width + m_height*m_height) / 2.0;
    return QRectF(m_center.getX() - diag, m_center.getY() - diag, diag * 2, diag * 2);
}
