#include "ArcPrimitive.h"

ArcPrimitive::ArcPrimitive(const PointPrimitive& center, double radius, double startAngle, double spanAngle)
    : m_center(center), m_radius(radius), m_startAngle(startAngle), m_spanAngle(spanAngle) {}

QRectF ArcPrimitive::getBoundingBox() const {
    // Упрощенный вариант bounding box (вся окружность)
    // Для точного расчета нужно учитывать углы, но для выделения рамкой этого достаточно
    return QRectF(m_center.getX() - m_radius, m_center.getY() - m_radius, m_radius * 2, m_radius * 2);
}
