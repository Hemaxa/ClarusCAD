#pragma once

#include "Point.h"

class Segment
{
public:
    Segment(const Point& start = Point(), const Point& end = Point());

    const Point& getStart() const;
    void setStart(const Point& point);

    const Point& getEnd() const;
    void setEnd(const Point& point);

private:
    Point m_start;
    Point m_end;
};
