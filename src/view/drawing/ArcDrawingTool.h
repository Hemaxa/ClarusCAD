//ArcDrawingTool - отрисовщик объекта "Дуга"

#pragma once

#include "BaseDrawingTool.h"

class ArcDrawingTool : public BaseDrawingTool
{
public:
    /**
     * @brief Отрисовать дугу.
     * @param painter QPainter.
     * @param primitive Указатель на ArcPrimitive.
     * @param isSelected Флаг выделения.
     */
    void draw(QPainter& painter, BasePrimitive* primitive, bool isSelected = false) const override;
};
