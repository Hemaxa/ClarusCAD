//ArcPrimitive - модель дуги

#pragma once
#include "BasePrimitive.h"
#include "PointPrimitive.h"

class ArcPrimitive : public BasePrimitive {
public:
    /**
     * @brief Конструктор дуги.
     * @param center Центр дуги.
     * @param radius Радиус дуги.
     * @param startAngle Начальный угол в градусах.
     * @param spanAngle Угловой размер сектора в градусах (может быть отрицательным).
     */
    ArcPrimitive(const PointPrimitive& center, double radius, double startAngle, double spanAngle);

    PrimitiveType getType() const override { return PrimitiveType::Arc; }
    QString getTypeName() const override { return "Дуга"; }

    // --- Smart Model ---
    void draw(QPainter& painter, bool isSelected) const override;
    QRectF getBoundingBox() const override;
    bool hitTest(const QPointF& point, double tolerance) const override;
    bool intersects(const QRectF& rect) const override;
    bool inside(const QRectF& rect) const override;
    QVector<QPointF> getSnapPoints() const override;

    /**
     * @brief Получить центр дуги.
     */
    PointPrimitive getCenter() const { return m_center; }

    /**
     * @brief Установить центр дуги.
     */
    void setCenter(const PointPrimitive& c) { m_center = c; }

    /**
     * @brief Получить радиус дуги.
     */
    double getRadius() const { return m_radius; }

    /**
     * @brief Установить радиус дуги.
     */
    void setRadius(double r) { m_radius = r; }

    /**
     * @brief Получить начальный угол (градусы).
     */
    double getStartAngle() const { return m_startAngle; }

    /**
     * @brief Установить начальный угол (градусы).
     */
    void setStartAngle(double a) { m_startAngle = a; }

    /**
     * @brief Получить угловой размер (градусы).
     */
    double getSpanAngle() const { return m_spanAngle; }

    /**
     * @brief Установить угловой размер (градусы).
     */
    void setSpanAngle(double a) { m_spanAngle = a; }

private:
    PointPrimitive m_center; ///< Центр дуги
    double m_radius;         ///< Радиус
    double m_startAngle;     ///< Начальный угол (градусы)
    double m_spanAngle;      ///< Угловой размер (градусы)
};
