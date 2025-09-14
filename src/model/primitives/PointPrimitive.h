//PointPrimitive - класс хранилища данных объекта "Точка"

#pragma once

#include "BasePrimitive.h"

class PointPrimitive : public BasePrimitive
{

public:
    //конструктор с параметрами по умолчанию
    PointPrimitive(double x = 0.0, double y = 0.0);

    //переопределение метода получения типа для объекта "Точка"
    PrimitiveType getType() const override { return PrimitiveType::Point; };

    //геттеры и сеттеры
    double getX() const;
    void setX(double x);

    double getY() const;
    void setY(double y);

private:
    //поля хранения
    double m_x;
    double m_y;
};
