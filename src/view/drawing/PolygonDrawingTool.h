//PolygonDrawingTool - инструмент отрисовки многоугольника

#pragma once

#include "BaseDrawingTool.h"

class PolygonDrawingTool : public BaseDrawingTool
{
public:
    void draw(QPainter& painter, BasePrimitive* primitive, bool isSelected) const override;
};
