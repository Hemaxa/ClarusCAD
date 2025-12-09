#pragma once

#include "BaseDrawingTool.h"

// Класс должен наследоваться от BaseDrawingTool
class RectangleDrawingTool : public BaseDrawingTool
{
public:
    // Объявление метода draw
    void draw(QPainter& painter, BasePrimitive* primitive, bool isSelected = false) const override;
};
