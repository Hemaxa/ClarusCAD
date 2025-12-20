//PolygonDrawingTool - инструмент отрисовки многоугольника

#pragma once

#include "BaseDrawingTool.h"

class PolygonDrawingTool : public BaseDrawingTool
{
public:
    /**
     * @brief Отрисовать многоугольник.
     * @param painter QPainter.
     * @param primitive Указатель на PolygonPrimitive.
     * @param isSelected Флаг выделения.
     */
    void draw(QPainter& painter, BasePrimitive* primitive, bool isSelected) const override;
};
