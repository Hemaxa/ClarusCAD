#include "SegmentPrimitive.h"

SegmentPrimitive::SegmentPrimitive(const PointPrimitive& start, const PointPrimitive& end) : m_start(start), m_end(end) {}

const PointPrimitive& SegmentPrimitive::getStart() const {
    return m_start;
}

void SegmentPrimitive::setStart(const PointPrimitive& point) {
    m_start = point;
}

const PointPrimitive& SegmentPrimitive::getEnd() const {
    return m_end;
}

void SegmentPrimitive::setEnd(const PointPrimitive& point) {
    m_end = point;
}

QRectF SegmentPrimitive::getBoundingBox() const
{
    qreal minX = std::min(m_start.getX(), m_end.getX());
    qreal maxX = std::max(m_start.getX(), m_end.getX());
    qreal minY = std::min(m_start.getY(), m_end.getY());
    qreal maxY = std::max(m_start.getY(), m_end.getY());
    return QRectF(QPointF(minX, minY), QPointF(maxX, maxY));
}
