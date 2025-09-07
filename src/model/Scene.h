#pragma once

#include "Point.h"

#include <vector>

class Scene
{

public:
    Scene();

    void addPoint(const Point& point);
    const std::vector<Point>& getPoints() const;

private:
    std::vector<Point> m_points;
};
