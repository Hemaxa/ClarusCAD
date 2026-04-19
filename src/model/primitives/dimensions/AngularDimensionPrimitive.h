// AngularDimensionPrimitive.h - класс углового размера

#pragma once

#include "BaseDimensionPrimitive.h"

class AngularDimensionPrimitive : public BaseDimensionPrimitive {
public:
    PrimitiveType getType() const override { return PrimitiveType::AngularDimension; }
    QString getTypeName() const override { return "Угловой размер"; }

    void setCenterPoint(const QPointF& point) { m_centerPoint = point; }
    QPointF getCenterPoint() const { return m_centerPoint; }

    void setStartPoint(const QPointF& point) { m_startPoint = point; }
    QPointF getStartPoint() const { return m_startPoint; }

    void setEndPoint(const QPointF& point) { m_endPoint = point; }
    QPointF getEndPoint() const { return m_endPoint; }

    void setArcPoint(const QPointF& point) { m_arcPoint = point; }
    QPointF getArcPoint() const { return m_arcPoint; }
    void setEdgeAssociation(BasePrimitive* first, int firstEdge, BasePrimitive* second, int secondEdge) {
        m_firstSource = first; m_firstEdgeIndex = firstEdge; m_secondSource = second; m_secondEdgeIndex = secondEdge;
    }
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
    QPointF m_startPoint;
    QPointF m_endPoint;
    QPointF m_arcPoint;
    BasePrimitive* m_firstSource = nullptr;
    BasePrimitive* m_secondSource = nullptr;
    int m_firstEdgeIndex = -1;
    int m_secondEdgeIndex = -1;
};
