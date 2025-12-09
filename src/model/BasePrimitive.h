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
    virtual void setLineType(int typeId) { m_lineTypeId = typeId; }
    virtual int getLineType() const { return m_lineTypeId; }

    //перегрузка метода задания типа линии для пользовательских стилей
    void setLineType(LineType type) { m_lineTypeId = static_cast<int>(type); }

private:
    QString m_name; //имя примитива
    QColor m_color = Qt::white; //цвет примитива
    int m_lineTypeId = static_cast<int>(LineType::SolidThin); //тип линии примитва
};
