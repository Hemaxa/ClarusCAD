//PointPrimitive - класс-контейнер для хранения координат точки
//не является примитивом сцены, просто хранит координаты (x, y)

#pragma once

#include "EnumManager.h"

#include <QRectF>
#include <QPointF>

class PointPrimitive
{

public:
    /**
     * @brief Конструктор точки.
     * @param x Координата X.
     * @param y Координата Y.
     */
    PointPrimitive(double x = 0.0, double y = 0.0);

    //получение типа (для совместимости, но это НЕ примитив сцены)
    PrimitiveType getType() const { return PrimitiveType::Point; }
    QString getTypeName() const { return "Точка"; }

    //получение габаритов (точка не имеет размера)
    QRectF getBoundingBox() const;

    /**
     * @brief Установить глобальную единицу измерения углов (Градусы/Радианы).
     */
    static void setAngleUnit(AngleUnit unit);

    /**
     * @brief Получить текущую единицу измерения углов.
     */
    static AngleUnit getAngleUnit();

    //геттеры и сеттеры
    //декартова система координат
    double getX() const;
    void setX(double x);
    double getY() const;
    void setY(double y);

    //полярная система координат
    /**
     * @brief Получить полярный радиус.
     */
    double getRadius() const;

    /**
     * @brief Получить полярный угол (в текущих единицах).
     */
    double getAngle() const;

    /**
     * @brief Установить координаты через полярные значения.
     * @param radius Полярный радиус.
     * @param angle Полярный угол.
     */
    void setPolar(double radius, double angle);

    /**
     * @brief Конвертировать в QPointF.
     */
    QPointF toPointF() const { return QPointF(m_x, m_y); }

    /**
     * @brief Создать из QPointF.
     */
    static PointPrimitive fromPointF(const QPointF& pt) { return PointPrimitive(pt.x(), pt.y()); }

private:
    //поля хранения
    double m_x;
    double m_y;

    //настройка единицы измерения углов
    static AngleUnit s_angleUnit;
};
