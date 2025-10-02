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
