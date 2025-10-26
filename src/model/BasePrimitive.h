//BasePrimitive - базовый класс для всех хранилищ данных объектов

#pragma once

#include "EnumManager.h"

#include <QColor> //для хранения цвета объекта
#include <QObject>

class BasePrimitive
{

public:
    //виртуальные деструктор и метод получения типа примитива
    virtual ~BasePrimitive() = default;
    virtual PrimitiveType getType() const { return PrimitiveType::Generic; }

    //виртуальные методы задания и получения цвета примитива
    virtual void setColor(const QColor& color) { m_color = color; }
    virtual QColor getColor() const { return m_color; }

    //виртуальные методы задания и получения типа линии примитива
    virtual void setLineType(LineType type) { m_lineType = type; }
    virtual LineType getLineType() const { return m_lineType; }

private:
    QColor m_color = Qt::white; //цвет примитива
    LineType m_lineType = LineType::Solid; //тип линии примитва
};
