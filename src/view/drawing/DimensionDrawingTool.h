// DimensionDrawingTool - универсальный отрисовщик размерных примитивов

#pragma once

#include "BaseDrawingTool.h"

class DimensionDrawingTool : public BaseDrawingTool
{
public:
    void draw(QPainter& painter, BasePrimitive* primitive, bool isSelected = false) const override;
};
