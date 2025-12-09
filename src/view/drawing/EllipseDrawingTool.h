#pragma once
#include "BaseDrawingTool.h"

class EllipseDrawingTool : public BaseDrawingTool {
public:
    void draw(QPainter& painter, BasePrimitive* primitive, bool isSelected = false) const override;
};
