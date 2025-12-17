//CirclePrimitive.h - модель окружности

#pragma once

#include "BasePrimitive.h"
#include "PointPrimitive.h"
#include <QRectF>

class CirclePrimitive : public BasePrimitive
{
public:
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
