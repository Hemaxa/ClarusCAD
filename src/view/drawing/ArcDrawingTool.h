//ArcDrawingTool - отрисовщик объекта "Дуга"

#pragma once

#include "BaseDrawingTool.h"

class ArcDrawingTool : public BaseDrawingTool
{
public:
    // Реализация метода отрисовки
    void draw(QPainter& painter, BasePrimitive* primitive, bool isSelected = false) const override;
};
