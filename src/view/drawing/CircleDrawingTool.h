//CircleDrawingTool - отрисовщик объекта "Окружность"

#pragma once

#include "BaseDrawingTool.h"

//наследуется от базового класса BaseDrawingTool
class CircleDrawingTool : public BaseDrawingTool
{
public:
    /**
     * @brief Отрисовать окружность.
     * @param painter QPainter.
     * @param primitive Указатель на CirclePrimitive.
     * @param isSelected Флаг выделения.
     */
    void draw(QPainter& painter, BasePrimitive* primitive, bool isSelected = false) const override;
};
