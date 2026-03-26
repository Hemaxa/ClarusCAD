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
    /**
     * @brief Конструктор сцены.
     */
    Scene();

    /**
     * @brief Добавить примитив в сцену.
     * @param primitive Уникальный указатель на добавляемый примитив.
     * Сцена берет на себя владение объектом.
     */
    void addPrimitive(std::unique_ptr<BasePrimitive> primitive);

    /**
     * @brief Удалить примитив из сцены.
     * @param primitive Указатель на удаляемый примитив.
     */
    void removePrimitive(BasePrimitive* primitive);

    /**
     * @brief Очистить сцену от всех объектов.
     */
    void clear();

    /**
     * @brief Получить список всех примитивов сцены.
     * @return Константная ссылка на вектор уникальных указателей примитивов.
     */
    const std::vector<std::unique_ptr<BasePrimitive>>& getPrimitives() const;

private:
    std::vector<std::unique_ptr<BasePrimitive>> m_primitives; ///< Хранилище всех объектов
    QMap<PrimitiveType, int> m_primitiveCounters;             ///< Счетчики объектов для генерации имен
};
