//CirclePrimitive.h - модель окружности

#pragma once

#include "BasePrimitive.h"
#include "PointPrimitive.h"
#include <QRectF>

class CirclePrimitive : public BasePrimitive
{
public:
    /**
     * @brief Конструктор окружности.
     * @param center Центр окружности.
     * @param radius Радиус окружности.
     */
    CirclePrimitive(const PointPrimitive& center, double radius);

    PrimitiveType getType() const override { return PrimitiveType::Circle; };
    QString getTypeName() const override { return "Окружность"; }

    // --- Реализация Smart Model ---
    void draw(QPainter& painter, bool isSelected) const override;
    QRectF getBoundingBox() const override;

    // Проверка клика по контуру окружности
    bool hitTest(const QPointF& point, double tolerance) const override;
    bool intersects(const QRectF& rect) const override;
    bool inside(const QRectF& rect) const override;
    QVector<QPointF> getSnapPoints() const override;

    // Геттеры/Сеттеры

    /**
     * @brief Получить центр окружности.
     */
    const PointPrimitive& getCenter() const;

    /**
     * @brief Установить центр окружности.
     */
    void setCenter(const PointPrimitive& center);

    /**
     * @brief Получить радиус.
     */
    double getRadius() const;

    /**
     * @brief Установить радиус.
     */
    void setRadius(double radius);

    /**
     * @brief Получить диаметр.
     */
    double getDiameter() const;

    /**
     * @brief Установить диаметр (меняет радиус).
     */
    void setDiameter(double diameter);

private:
    PointPrimitive m_center; ///< Центр окружности
    double m_radius;         ///< Радиус
};
