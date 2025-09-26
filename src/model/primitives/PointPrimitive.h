//PointPrimitive - класс хранилища данных объекта "Точка"

#pragma once

#include "BasePrimitive.h"
#include "EnumManager.h"

class PointPrimitive : public BasePrimitive
{

public:
    //конструктор с параметрами по умолчанию
    PointPrimitive(double x = 0.0, double y = 0.0);

    //переопределение метода получения типа для объекта "Точка"
    PrimitiveType getType() const override { return PrimitiveType::Point; };

    //методы для установки глобальных единиц измерения углов
    static void setAngleUnit(AngleUnit unit);
    static AngleUnit getAngleUnit();

    //геттеры и сеттеры
    //декартова система координат
    double getX() const;
    void setX(double x);
    double getY() const;
    void setY(double y);

    //полярная система координат
    double getRadius() const;
    double getAngle() const;

    //метод конвертации полярных координат в декартовы
    void setPolar(double radius, double angle);

private:
    //поля хранения
    double m_x;
    double m_y;

    //настройка единицы измерения углов
    static AngleUnit s_angleUnit;
};
