//CirclePrimitive - класс хранилища данных объекта "Окружность"

#pragma once

#include "BasePrimitive.h"
#include "PointPrimitive.h"

#include <QRectF>

class CirclePrimitive : public BasePrimitive
{
public:
    //конструктор
    CirclePrimitive(const PointPrimitive& center, double radius);

    //переопределение методов получения типа для объекта "Окружность"
    PrimitiveType getType() const override { return PrimitiveType::Circle; };
    QString getTypeName() const override { return "Окружность"; }

    //переопределение метода получения габаритов
    QRectF getBoundingBox() const override;

    //геттеры и сеттеры
    const PointPrimitive& getCenter() const;
    void setCenter(const PointPrimitive& center);

    double getRadius() const;
    void setRadius(double radius);

    double getDiameter() const;
    void setDiameter(double diameter);

private:
    PointPrimitive m_center;
    double m_radius;
};
