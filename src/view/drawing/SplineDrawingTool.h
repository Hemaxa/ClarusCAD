//SplineDrawingTool - инструмент отрисовки сплайна

#pragma once

#include "BaseDrawingTool.h"

class SplineDrawingTool : public BaseDrawingTool
{
public:
    void draw(QPainter& painter, BasePrimitive* primitive, bool isSelected) const override;
};
