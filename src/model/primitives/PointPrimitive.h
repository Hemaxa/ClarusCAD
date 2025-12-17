//PointPrimitive - класс-контейнер для хранения координат точки
//НЕ является примитивом сцены, просто хранит координаты (x, y)

#pragma once

#include "EnumManager.h"

#include <QRectF>
#include <QPointF>

class PointPrimitive
{

public:
    //конструктор с параметрами по умолчанию
    PointPrimitive(double x = 0.0, double y = 0.0);

    //получение типа (для совместимости, но это НЕ примитив сцены)
    PrimitiveType getType() const { return PrimitiveType::Point; }
    QString getTypeName() const { return "Точка"; }

    //получение габаритов (точка не имеет размера)
    QRectF getBoundingBox() const;

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

    //конвертация в QPointF для удобства
    QPointF toPointF() const { return QPointF(m_x, m_y); }

    //создание из QPointF
    static PointPrimitive fromPointF(const QPointF& pt) { return PointPrimitive(pt.x(), pt.y()); }

private:
    //поля хранения
    double m_x;
    double m_y;

    //настройка единицы измерения углов
    static AngleUnit s_angleUnit;
};
