//SegmentDrawingTool - отрисовщик объекта "Отрезок"

#pragma once

#include "BaseDrawingTool.h"

//наслдедуется от базового класса BaseDrawingTool
class SegmentDrawingTool : public BaseDrawingTool
{

public:
    /**
     * @brief Отрисовать отрезок.
     * @param painter QPainter.
     * @param primitive Указатель на SegmentPrimitive.
     * @param isSelected Флаг выделения.
     */
    void draw(QPainter& painter, BasePrimitive* primitive, bool isSelected = false) const override;
};
