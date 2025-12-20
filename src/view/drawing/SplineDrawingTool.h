//SplineDrawingTool - инструмент отрисовки сплайна

#pragma once

#include "BaseDrawingTool.h"

class SplineDrawingTool : public BaseDrawingTool
{
public:
    /**
     * @brief Отрисовать сплайн.
     * @param painter QPainter.
     * @param primitive Указатель на SplinePrimitive.
     * @param isSelected Флаг выделения.
     */
    void draw(QPainter& painter, BasePrimitive* primitive, bool isSelected) const override;
};
