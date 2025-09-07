#include "ViewportWidget.h"
#include "Scene.h"
#include "Segment.h"

#include <QPainter>
#include <cmath>

ViewportWidget::ViewportWidget(QWidget *parent) : QWidget(parent)
{
}

void ViewportWidget::setScene(Scene* scene)
{
    m_scene = scene;
}

void ViewportWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setBrush(QColor(45, 45, 45));
    painter.drawRect(rect());

    if (!m_scene) {
        return;
    }

    painter.setPen(Qt::white);

    // 1. Рисуем точки
    painter.setBrush(Qt::white);
    for (const auto& point : m_scene->getPoints()) {
        painter.drawEllipse(QPointF(point.x(), point.y()), 3, 3);
    }

    // 2. Рисуем отрезки по алгоритму Брезенхема
    for (const auto& segment : m_scene->getSegments()) {
        int x0 = static_cast<int>(segment.getStart().x());
        int y0 = static_cast<int>(segment.getStart().y());
        int x1 = static_cast<int>(segment.getEnd().x());
        int y1 = static_cast<int>(segment.getEnd().y());

        int dx = std::abs(x1 - x0);
        int dy = -std::abs(y1 - y0);

        int sx = x0 < x1 ? 1 : -1;
        int sy = y0 < y1 ? 1 : -1;

        int err = dx + dy;

        while (true) {
            painter.drawPoint(x0, y0); // Рисуем текущий пиксель
            if (x0 == x1 && y0 == y1) {
                break;
            }
            int e2 = 2 * err;
            if (e2 >= dy) {
                err += dy;
                x0 += sx;
            }
            if (e2 <= dx) {
                err += dx;
                y0 += sy;
            }
        }
    }
}
