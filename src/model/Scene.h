//Scene - центральное хранилище всех объектов в приложении

#pragma once

#include "EnumManager.h"
#include "BasePrimitive.h"

#include <vector>
#include <memory>
#include <QMap>

class Scene
{

public:
    //конструктор
    Scene();

    //метод добавления примитива в сцену
    void addPrimitive(std::unique_ptr<BasePrimitive> primitive);

    //метод удаления примитива из сцены
    void removePrimitive(BasePrimitive* primitive);

    //метод получения доступа ко всем примитивам
    const std::vector<std::unique_ptr<BasePrimitive>>& getPrimitives() const;

private:
    //хранилище всех объектов
    std::vector<std::unique_ptr<BasePrimitive>> m_primitives;

    //хранилище количетсва всех объектов для генерации уникальных имен
    QMap<PrimitiveType, int> m_primitiveCounters;
};
