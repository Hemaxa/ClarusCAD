#include "Scene.h"

Scene::Scene() {}

void Scene::addPrimitive(std::unique_ptr<BasePrimitive> primitive)
{
    //защита от nullptr
    if (!primitive) return;

    //получение типа примитива
    PrimitiveType type = primitive->getType();

    //увеличение счетчика для этого типа
    int newId = ++m_primitiveCounters[type];

    //создание и установка имемни
    QString newName = QString("%1 %2").arg(primitive->getTypeName()).arg(newId);
    primitive->setName(newName);

    //добавление примитива
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

void Scene::clear()
{
    m_primitives.clear();
    m_primitiveCounters.clear();
}

const std::vector<std::unique_ptr<BasePrimitive>>& Scene::getPrimitives() const
{
    return m_primitives;
}
