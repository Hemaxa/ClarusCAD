#include "Scene.h"

Scene::Scene() {}

void Scene::addPrimitive(std::unique_ptr<BasePrimitive> primitive)
{
    m_primitives.push_back(std::move(primitive));
}

const std::vector<std::unique_ptr<BasePrimitive>>& Scene::getPrimitives() const
{
    return m_primitives;
}
