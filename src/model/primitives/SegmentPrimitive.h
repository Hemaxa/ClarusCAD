//SegmentPrimitive - класс хранилища данных объекта "Отрезок"

#pragma once

#include "BasePrimitive.h"
#include "PointPrimitive.h"

class SegmentPrimitive : public BasePrimitive
{

public:
    //конструктор
    SegmentPrimitive(const PointPrimitive& start, const PointPrimitive& end);

    //переопределение метода получения типа для объекта "Отрезок"
    PrimitiveType getType() const override { return PrimitiveType::Segment; };

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
