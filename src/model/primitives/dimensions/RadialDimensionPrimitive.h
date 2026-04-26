// RadialDimensionPrimitive.h - класс радиального/диаметрального размера

#pragma once

#include "BaseDimensionPrimitive.h"

class RadialDimensionPrimitive : public BaseDimensionPrimitive {
public:
    PrimitiveType getType() const override { return PrimitiveType::RadialDimension; }
    QString getTypeName() const override { return m_isDiameter ? "Диаметральный размер" : "Радиальный размер"; }

    void setCenterPoint(const QPointF& point) { m_centerPoint = point; }
    QPointF getCenterPoint() const { return m_centerPoint; }

    void setRadiusPoint(const QPointF& point) { m_radiusPoint = point; }
    QPointF getRadiusPoint() const { return m_radiusPoint; }

    void setDimensionLinePos(const QPointF& pos);
    QPointF getDimensionLinePos() const { return m_dimensionLinePos; }

    void setDiameterMode(bool isDiameter) { m_isDiameter = isDiameter; recalculateValue(); }
    bool isDiameterMode() const { return m_isDiameter; }
    void setAssociatedPrimitive(BasePrimitive* primitive, double angle) { m_associatedPrimitive = primitive; m_associationAngle = angle; }
    BasePrimitive* getAssociatedPrimitive() const { return m_associatedPrimitive; }
    void setAssociationAngle(double angle) { m_associationAngle = angle; }
    double getAssociationAngle() const { return m_associationAngle; }
    void updateFromAssociation();

    void recalculateValue() override;
    bool applyMeasuredValueOverride(double value) override;

    QVector<QPointF> getSnapPoints() const override;
    void draw(QPainter& painter, bool isSelected) const override;
    QRectF getBoundingBox() const override;
    bool hitTest(const QPointF& point, double tolerance) const override;
    bool intersects(const QRectF& rect) const override;
    bool inside(const QRectF& rect) const override;
    QPointF getDefaultTextAnchor() const override;
    QPointF constrainTextAnchor(const QPointF& pos) const override;
    QVector<QPointF> getEditGripPoints() const override;
    void moveGripPoint(int index, const QPointF& newPos) override;
    void setColor(const QColor& color) override {
        BasePrimitive::setColor(color);
        m_style.extensionLineColor = color;
        m_style.dimensionLineColor = color;
        m_style.textColor = color;
    }

private:
    QPointF m_centerPoint;
    QPointF m_radiusPoint;
    QPointF m_dimensionLinePos;
    bool m_isDiameter = false;
    BasePrimitive* m_associatedPrimitive = nullptr;
    double m_associationAngle = 0.0;
};
