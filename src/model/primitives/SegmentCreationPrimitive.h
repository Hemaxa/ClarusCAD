#pragma once

#include "BasePrimitive.h"
#include "PointCreationPrimitive.h"

class SegmentCreationPrimitive : public BasePrimitive
{

public:
    SegmentCreationPrimitive(const PointCreationPrimitive& start = PointCreationPrimitive(), const PointCreationPrimitive& end = PointCreationPrimitive());

    PrimitiveType getType() const override { return PrimitiveType::Segment; }

    const PointCreationPrimitive& getStart() const;
    void setStart(const PointCreationPrimitive& point);

    const PointCreationPrimitive& getEnd() const;
    void setEnd(const PointCreationPrimitive& point);

private:
    PointCreationPrimitive m_start;
    PointCreationPrimitive m_end;
};
