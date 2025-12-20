//EllipseDrawingTool - отрисовщик объекта "Эллипс"

#pragma once
#include "BaseDrawingTool.h"

class EllipseDrawingTool : public BaseDrawingTool {
public:
    /**
     * @brief Отрисовать эллипс.
     * @param painter QPainter.
     * @param primitive Указатель на EllipsePrimitive.
     * @param isSelected Флаг выделения.
     */
    void draw(QPainter& painter, BasePrimitive* primitive, bool isSelected = false) const override;
};
