//SegmentPrimitive - класс хранилища данных объекта "Отрезок"

#pragma once

#include "BasePrimitive.h"
#include "PointPrimitive.h"

#include <QRectF>

class SegmentPrimitive : public BasePrimitive
{

public:
    //конструктор
    SegmentPrimitive(const PointPrimitive& start, const PointPrimitive& end);

    //переопределение методов получения типа для объекта "Отрезок"
    PrimitiveType getType() const override { return PrimitiveType::Segment; };
    QString getTypeName() const override { return "Отрезок"; }

    //переопределение метода получения габаритов объекта "Отрезок"
    QRectF getBoundingBox() const override;

    //геттер и сеттер начальной точки
    const PointPrimitive& getStart() const;
    void setStart(const PointPrimitive& point);

    //геттер и сеттер конечной точки
    const PointPrimitive& getEnd() const;
    void setEnd(const PointPrimitive& point);

private:
    //поля хранения
    PointPrimitive m_start;
    PointPrimitive m_end;
};
