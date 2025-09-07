#include "Scene.h"

Scene::Scene()
{
}

void Scene::addPoint(const Point& point)
{
    m_points.push_back(point);
}

const std::vector<Point>& Scene::getPoints() const
{
    return m_points;
}
