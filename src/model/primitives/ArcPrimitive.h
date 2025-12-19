//ArcPrimitive - модель дуги

#pragma once
#include "BasePrimitive.h"
#include "PointPrimitive.h"

class ArcPrimitive : public BasePrimitive {
public:
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

    PointPrimitive getCenter() const { return m_center; }
    void setCenter(const PointPrimitive& c) { m_center = c; }

    double getRadius() const { return m_radius; }
    void setRadius(double r) { m_radius = r; }

    double getStartAngle() const { return m_startAngle; }
    void setStartAngle(double a) { m_startAngle = a; }

    double getSpanAngle() const { return m_spanAngle; }
    void setSpanAngle(double a) { m_spanAngle = a; }

private:
    PointPrimitive m_center;
    double m_radius;
    double m_startAngle; // градусы
    double m_spanAngle;  // градусы
};
