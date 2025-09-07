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

void Scene::addSegment(const Segment& segment)
{
    m_segments.push_back(segment);
}

const std::vector<Segment>& Scene::getSegments() const
{
    return m_segments;
}
