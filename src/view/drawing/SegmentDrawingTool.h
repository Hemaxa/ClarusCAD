//SegmentDrawingTool - отрисовщик объекта "Отрезок"

#pragma once

#include "BaseDrawingTool.h"

//наслдедуется от базового класса BaseDrawingTool
class SegmentDrawingTool : public BaseDrawingTool
{

public:
    //переопределние метода draw
    void draw(QPainter& painter, BasePrimitive* primitive, bool isSelected = false) const override;
};
