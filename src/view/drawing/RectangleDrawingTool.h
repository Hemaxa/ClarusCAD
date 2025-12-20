//RectangleDrawingTool - отрисовщик объекта "Прямоугольник"

#pragma once

#include "BaseDrawingTool.h"

// Класс должен наследоваться от BaseDrawingTool
class RectangleDrawingTool : public BaseDrawingTool
{
public:
    /**
     * @brief Отрисовать прямоугольник.
     * @param painter QPainter.
     * @param primitive Указатель на RectanglePrimitive.
     * @param isSelected Флаг выделения.
     */
    void draw(QPainter& painter, BasePrimitive* primitive, bool isSelected = false) const override;
};
