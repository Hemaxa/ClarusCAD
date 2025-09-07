#pragma once

#include "Point.h"
#include "Segment.h"

#include <vector>

class Scene
{

public:
    Scene();

    void addPoint(const Point& point);
    const std::vector<Point>& getPoints() const;

    void addSegment(const Segment& segment);
    const std::vector<Segment>& getSegments() const;

private:
    std::vector<Point> m_points;
    std::vector<Segment> m_segments;
};
