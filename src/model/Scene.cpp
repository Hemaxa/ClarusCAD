#include "Scene.h"

Scene::Scene() {}

void Scene::addPrimitive(std::unique_ptr<BasePrimitive> primitive)
{
    m_primitives.push_back(std::move(primitive));
}

void Scene::removePrimitive(BasePrimitive* primitive)
{
    //примитив ищется в векторе и удаляется
    m_primitives.erase(
        std::remove_if(m_primitives.begin(), m_primitives.end(),
                       [primitive](const std::unique_ptr<BasePrimitive>& p) {
                           return p.get() == primitive;
                       }),
        m_primitives.end());
}

const std::vector<std::unique_ptr<BasePrimitive>>& Scene::getPrimitives() const
{
    return m_primitives;
}
