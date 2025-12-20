//SegmentPrimitive - класс хранилища данных объекта "Отрезок"

#pragma once

#include "BasePrimitive.h"
#include "PointPrimitive.h"

class SegmentPrimitive : public BasePrimitive
{

public:
    /**
     * @brief Конструктор отрезка.
     * @param start Начальная точка.
     * @param end Конечная точка.
     */
    SegmentPrimitive(const PointPrimitive& start, const PointPrimitive& end);

    //переопределение методов получения типа для объекта "Отрезок"
    PrimitiveType getType() const override { return PrimitiveType::Segment; };
    QString getTypeName() const override { return "Отрезок"; }

    // --- Реализация Smart Model ---
    void draw(QPainter& painter, bool isSelected) const override;
    QRectF getBoundingBox() const override;
    bool hitTest(const QPointF& point, double tolerance) const override;
    bool intersects(const QRectF& rect) const override;
    bool inside(const QRectF& rect) const override;
    QVector<QPointF> getSnapPoints() const override;

    /**
     * @brief Получить начальную точку.
     */
    const PointPrimitive& getStart() const;

    /**
     * @brief Установить начальную точку.
     */
    void setStart(const PointPrimitive& point);

    /**
     * @brief Получить конечную точку.
     */
    const PointPrimitive& getEnd() const;

    /**
     * @brief Установить конечную точку.
     */
    void setEnd(const PointPrimitive& point);

private:
    //поля хранения
    PointPrimitive m_start;
    PointPrimitive m_end;
};
