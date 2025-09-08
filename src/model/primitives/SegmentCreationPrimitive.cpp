#include "SegmentCreationPrimitive.h"

SegmentCreationPrimitive::SegmentCreationPrimitive(const PointCreationPrimitive& start, const PointCreationPrimitive& end)
    : m_start(start), m_end(end) {}

const PointCreationPrimitive& SegmentCreationPrimitive::getStart() const {
    return m_start;
}

void SegmentCreationPrimitive::setStart(const PointCreationPrimitive& point) {
    m_start = point;
}

const PointCreationPrimitive& SegmentCreationPrimitive::getEnd() const {
    return m_end;
}

void SegmentCreationPrimitive::setEnd(const PointCreationPrimitive& point) {
    m_end = point;
}
