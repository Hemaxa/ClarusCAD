#pragma once
#include "BasePrimitive.h"
#include "PointPrimitive.h"

class ArcPrimitive : public BasePrimitive {
public:
    ArcPrimitive(const PointPrimitive& center, double radius, double startAngle, double spanAngle);

    PrimitiveType getType() const override { return PrimitiveType::Arc; }
    QString getTypeName() const override { return "Дуга"; }
    QRectF getBoundingBox() const override;

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
    double m_startAngle; // В градусах
    double m_spanAngle;  // В градусах
};
