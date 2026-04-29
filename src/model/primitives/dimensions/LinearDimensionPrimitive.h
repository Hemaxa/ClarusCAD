// LinearDimensionPrimitive.h - класс линейного размера

#pragma once

#include "BaseDimensionPrimitive.h"
#include "../../../view/managers/SnapManager.h"
#include <QPointF>
#include <QPainter>

enum class LinearDimensionMode {
    Aligned,
    Horizontal,
    Vertical
};

/**
 * @brief Класс линейного размера
 */
class LinearDimensionPrimitive : public BaseDimensionPrimitive {
public:
    struct Attachment {
        BasePrimitive* source = nullptr;
        SnapType snapType = SnapType::None;
        int index = -1;
        double param = 0.0;
        QPointF fallback;
    };

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
    void setStartAttachment(const Attachment& a) { m_startAttachment = a; }
    void setEndAttachment(const Attachment& a) { m_endAttachment = a; }
    Attachment getStartAttachment() const { return m_startAttachment; }
    Attachment getEndAttachment() const { return m_endAttachment; }
    void updateFromAttachments();

    void setMode(LinearDimensionMode mode) { m_mode = mode; recalculateValue(); }
    LinearDimensionMode getMode() const { return m_mode; }
    void setTextPrefix(const QString& prefix) { m_textPrefix = prefix; }
    QString getTextPrefix() const { return m_textPrefix; }

    virtual void recalculateValue() override;
    bool applyMeasuredValueOverride(double value) override;

    // Переопределенные методы BasePrimitive
    virtual QVector<QPointF> getSnapPoints() const override;
    virtual void draw(QPainter& painter, bool isSelected) const override;
    virtual QRectF getBoundingBox() const override;
    virtual bool hitTest(const QPointF& point, double tolerance) const override;
    virtual bool intersects(const QRectF& rect) const override;
    virtual bool inside(const QRectF& rect) const override;
    QPointF getDefaultTextAnchor() const override;
    QPointF constrainTextAnchor(const QPointF& pos) const override;
    QVector<QPointF> getEditGripPoints() const override;
    void moveGripPoint(int index, const QPointF& newPos) override;
    virtual void setColor(const QColor& color) override {
        BasePrimitive::setColor(color);
        m_style.extensionLineColor = color;
        m_style.dimensionLineColor = color;
        m_style.textColor = color;
    }

private:
    QPointF m_startPoint;       ///< Первая точка измерения
    QPointF m_endPoint;         ///< Вторая точка измерения
    QPointF m_dimensionLinePos; ///< Положение самой размерной линии (точка клика, определяющая отступ)
    LinearDimensionMode m_mode = LinearDimensionMode::Aligned;
    Attachment m_startAttachment;
    Attachment m_endAttachment;
    QString m_textPrefix;
};
