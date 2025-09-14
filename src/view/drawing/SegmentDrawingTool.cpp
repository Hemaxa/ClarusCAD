#include "SegmentDrawingTool.h"
#include "SegmentPrimitive.h"
#include <QPainter>
#include <QPen>

void SegmentDrawingTool::draw(QPainter& painter, BasePrimitive* primitive) const
{
    //приведение базового типа к конкретному
    auto* segment = static_cast<SegmentPrimitive*>(primitive);

    //если приведение не удалось или пришел nullptr, прекращается выполнение
    if (!segment) return;


    //устанавливается перо для рисования
    painter.setPen(Qt::white);

    //реализация алгоритма Брезенхема
    int x1 = static_cast<int>(segment->getStart().getX());
    int y1 = static_cast<int>(segment->getStart().getY());
    int x2 = static_cast<int>(segment->getEnd().getX());
    int y2 = static_cast<int>(segment->getEnd().getY());

    const int deltaX = abs(x2 - x1);
    const int deltaY = abs(y2 - y1);
    const int signX = x1 < x2 ? 1 : -1;
    const int signY = y1 < y2 ? 1 : -1;

    int error = deltaX - deltaY;

    //цикл отрисовки пикселей
    while (x1 != x2 || y1 != y2) {
        painter.drawPoint(x1, y1);
        int error2 = error * 2;

        if (error2 > -deltaY) {
            error -= deltaY;
            x1 += signX;
        }
        if (error2 < deltaX) {
            error += deltaX;
            y1 += signY;
        }
    }
    //отрисовка последней точки
    painter.drawPoint(x2, y2);
}
