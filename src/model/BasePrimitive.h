//BasePrimitive - базовый класс для всех хранилищ данных объектов

#pragma once

#include "EnumManager.h"

#include <QColor> //для хранения цвета объекта

class BasePrimitive
{

public:
    //виртуальные деструктор и метод получения типа примитива
    virtual ~BasePrimitive() = default;
    virtual PrimitiveType getType() const { return PrimitiveType::Generic; }

    //виртуальные методы задания и получения цвета примитива
    virtual void setColor(const QColor& color) { m_color = color; }
    virtual QColor getColor() const { return m_color; }

private:
    QColor m_color = Qt::white; //цвет примитива
};
