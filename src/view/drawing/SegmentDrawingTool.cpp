#include "SegmentDrawingTool.h"
#include "SegmentPrimitive.h"
#include "LineStyleManager.h" // Подключаем менеджер

#include <QPainter>

// ВНИМАНИЕ: Это SegmentDrawingTool::draw, а НЕ SegmentCreationTool::onPaint
void SegmentDrawingTool::draw(QPainter& painter, BasePrimitive* primitive, bool isSelected) const
{
    auto* segment = static_cast<SegmentPrimitive*>(primitive);
    if (!segment) return;

    // Делегируем отрисовку менеджеру стилей
    LineStyleManager::instance().drawLine(
        painter,
        QPointF(segment->getStart().getX(), segment->getStart().getY()),
        QPointF(segment->getEnd().getX(), segment->getEnd().getY()),
        segment->getLineType(), // Тип линии берем из примитива
        segment->getColor(),
        isSelected
        );
}
