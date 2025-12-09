//BasePrimitive - базовый класс для всех хранилищ данных объектов

#pragma once

#include "EnumManager.h"

#include <QColor> //для хранения цвета объекта
#include <QString> //для хранения имени объекта
#include <QObject>
#include <QRectF>

class BasePrimitive
{

public:
    //виртуальные деструктор и методы получения типа примитива
    virtual ~BasePrimitive() = default;

    //виртуальные методы получения общей характеристики примитива
    virtual PrimitiveType getType() const { return PrimitiveType::Generic; }
    virtual QString getTypeName() const { return "Примитив"; }
    virtual QRectF getBoundingBox() const { return QRectF(); }

    //виртуальные методы задания и получения имени примитива
    virtual void setName(const QString& name) { m_name = name; }
    virtual QString getName() const { return m_name; }

    //виртуальные методы задания и получения цвета примитива
    virtual void setColor(const QColor& color) { m_color = color; }
    virtual QColor getColor() const { return m_color; }

    //виртуальные методы задания и получения типа линии примитива
    virtual void setLineType(LineType type) { m_lineType = type; }
    virtual LineType getLineType() const { return m_lineType; }

private:
    QString m_name; //имя примитива
    QColor m_color = Qt::white; //цвет примитива
    LineType m_lineType = LineType::SolidThin; //тип линии примитва
};
