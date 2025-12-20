//PolygonPrimitive - примитив многоугольника (правильный многоугольник)
//Параметры: центр, радиус, количество сторон, вписанный/описанный

#pragma once

#include "BasePrimitive.h"
#include "PointPrimitive.h"

// Тип построения многоугольника
/**
 * @brief Тип построения многоугольника.
 */
enum class PolygonType {
    Inscribed,  ///< Вписанный в окружность (вершины на окружности)
    Circumscribed ///< Описанный вокруг окружности (стороны касаются окружности)
};

class PolygonPrimitive : public BasePrimitive
{
public:
    /**
     * @brief Конструктор многоугольника.
     * @param center Центр.
     * @param radius Радиус (описанной или вписанной окружности).
     * @param sides Количество сторон (минимум 3).
     * @param type Тип (Вписанный/Описанный).
     * @param rotation Угол поворота.
     */
    PolygonPrimitive(const PointPrimitive& center, double radius, int sides = 6,
                     PolygonType type = PolygonType::Inscribed, double rotation = 0.0);

    // Переопределение типа
    PrimitiveType getType() const override { return PrimitiveType::Polygon; }
    QString getTypeName() const override { return "Многоугольник"; }

    // Smart Model методы
    void draw(QPainter& painter, bool isSelected) const override;
    QRectF getBoundingBox() const override;
    bool hitTest(const QPointF& point, double tolerance) const override;
    bool intersects(const QRectF& rect) const override;
    bool inside(const QRectF& rect) const override;
    QVector<QPointF> getSnapPoints() const override;

    // Геттеры/сеттеры

    /**
     * @brief Получить центр.
     */
    PointPrimitive getCenter() const { return m_center; }
    void setCenter(const PointPrimitive& c) { m_center = c; }

    /**
     * @brief Получить радиус.
     */
    double getRadius() const { return m_radius; }
    void setRadius(double r) { m_radius = r; }

    /**
     * @brief Получить количество сторон.
     */
    int getSides() const { return m_sides; }
    void setSides(int s) { m_sides = (s >= 3) ? s : 3; } // Минимум 3 стороны

    /**
     * @brief Получить тип построения.
     */
    PolygonType getPolygonType() const { return m_polygonType; }
    void setPolygonType(PolygonType t) { m_polygonType = t; }

    /**
     * @brief Получить угол поворота.
     */
    double getRotation() const { return m_rotation; }
    void setRotation(double r) { m_rotation = r; }

    /**
     * @brief Получить вычисленные вершины многоугольника.
     * @return Вектор точек вершин.
     */
    QVector<QPointF> getVertices() const;

private:
    PointPrimitive m_center;
    double m_radius;
    int m_sides;         ///< Количество сторон
    PolygonType m_polygonType;
    double m_rotation;   ///< Угол поворота в градусах
};
