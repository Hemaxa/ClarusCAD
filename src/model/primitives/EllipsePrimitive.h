//EllipsePrimitive.h

#pragma once
#include "BasePrimitive.h"
#include "PointPrimitive.h"

class EllipsePrimitive : public BasePrimitive {
public:
    EllipsePrimitive(const PointPrimitive& center, double rx, double ry, double rotation = 0.0);

    PrimitiveType getType() const override { return PrimitiveType::Ellipse; }
    QString getTypeName() const override { return "Эллипс"; }

    // --- Smart Model ---
    void draw(QPainter& painter, bool isSelected) const override;
    QRectF getBoundingBox() const override;
    bool hitTest(const QPointF& point, double tolerance) const override;
    bool intersects(const QRectF& rect) const override;
    bool inside(const QRectF& rect) const override;
    QVector<QPointF> getSnapPoints() const override;

    PointPrimitive getCenter() const { return m_center; }
    void setCenter(const PointPrimitive& c) { m_center = c; }

    double getRadiusX() const { return m_radiusX; }
    void setRadiusX(double r) { m_radiusX = r; }

    double getRadiusY() const { return m_radiusY; }
    void setRadiusY(double r) { m_radiusY = r; }

    double getRotation() const { return m_rotation; }
    void setRotation(double r) { m_rotation = r; }

private:
    PointPrimitive m_center;
    double m_radiusX;
    double m_radiusY;
    double m_rotation;
};
