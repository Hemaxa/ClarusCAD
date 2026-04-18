// LinearDimensionPrimitive.h - класс линейного размера

#pragma once

#include "BaseDimensionPrimitive.h"
#include <QPointF>
#include <QPainter>

/**
 * @brief Класс линейного размера
 */
class LinearDimensionPrimitive : public BaseDimensionPrimitive {
public:
    LinearDimensionPrimitive() = default;
    virtual ~LinearDimensionPrimitive() = default;

    PrimitiveType getType() const override { return PrimitiveType::LinearDimension; }
    QString getTypeName() const override { return "Линейный размер"; }

    void setStartPoint(const QPointF& point) { m_startPoint = point; }
    QPointF getStartPoint() const { return m_startPoint; }

    void setEndPoint(const QPointF& point) { m_endPoint = point; }
    QPointF getEndPoint() const { return m_endPoint; }

    void setDimensionLinePos(const QPointF& pos) { m_dimensionLinePos = pos; }
    QPointF getDimensionLinePos() const { return m_dimensionLinePos; }

    virtual void recalculateValue() override;

    // Переопределенные методы BasePrimitive
    virtual QVector<QPointF> getSnapPoints() const override;
    virtual void draw(QPainter& painter, bool isSelected) const override;
    virtual QRectF getBoundingBox() const override;
    virtual bool hitTest(const QPointF& point, double tolerance) const override;
    virtual bool intersects(const QRectF& rect) const override;
    virtual bool inside(const QRectF& rect) const override;
    virtual void setColor(const QColor& color) override {
        BasePrimitive::setColor(color);
        m_style.dimensionLineColor = color;
        m_style.textColor = color; // Раскомментируй, если хочешь, чтобы текст тоже красился
    }

private:
    QPointF m_startPoint;       ///< Первая точка измерения
    QPointF m_endPoint;         ///< Вторая точка измерения
    QPointF m_dimensionLinePos; ///< Положение самой размерной линии (точка клика, определяющая отступ)
};
