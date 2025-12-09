//CircleDrawingTool - отрисовщик объекта "Окружность"

#pragma once

#include "BaseDrawingTool.h"

//наследуется от базового класса BaseDrawingTool
class CircleDrawingTool : public BaseDrawingTool
{
public:
    //переопределение метода draw
    void draw(QPainter& painter, BasePrimitive* primitive, bool isSelected = false) const override;
};
