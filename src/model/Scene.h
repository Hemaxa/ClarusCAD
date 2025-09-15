//Scene - центральное хранилище всех объектов в приложении

#pragma once

#include "BasePrimitive.h"

#include <vector>
#include <memory>

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
};
