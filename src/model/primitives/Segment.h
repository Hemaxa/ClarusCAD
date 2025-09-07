#pragma once

#include "Point.h"
#include "BasePrimitive.h"

class Segment : public BasePrimitive
{

public:
    Segment(const Point& start = Point(), const Point& end = Point());

    const Point& getStart() const;
    void setStart(const Point& point);

    const Point& getEnd() const;
    void setEnd(const Point& point);

    PrimitiveType getType() const override { return PrimitiveType::Segment; }

private:
    Point m_start;
    Point m_end;
};
