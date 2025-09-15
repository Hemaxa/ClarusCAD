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
    //1) получение координат и приведение их к целым числам
    int x1 = static_cast<int>(segment->getStart().getX());
    int y1 = static_cast<int>(segment->getStart().getY());
    int x2 = static_cast<int>(segment->getEnd().getX());
    int y2 = static_cast<int>(segment->getEnd().getY());

    //2) вычисление "размаха" линии по осям
    const int deltaX = abs(x2 - x1);
    const int deltaY = abs(y2 - y1);

    //3) определение направления рисования
    const int signX = x1 < x2 ? 1 : -1;
    const int signY = y1 < y2 ? 1 : -1;

    //4) инициализация "ошибки"
    //ключевая переменная, которая помогает алгоритму решить, когда нужно сдвинуться по оси
    int error = deltaX - deltaY;

    //5) основной цикл, который работает, пока текущая точка не достигнет конечной
    while (x1 != x2 || y1 != y2) {
        //рисуется пиксель в каждой точке
        painter.drawPoint(x1, y1);

        //ошибка удваивается для удобства сравнения
        int error2 = error * 2;

        //6) принимается решение о следующем шаге
        if (error2 > -deltaY) {
            error -= deltaY;
            x1 += signX;
        }
        if (error2 < deltaX) {
            error += deltaX;
            y1 += signY;
        }
    }
    //7) цикл `while` завершается, не нарисовав последнюю точку, поэтому она отрисовывается
    painter.drawPoint(x2, y2);
}
