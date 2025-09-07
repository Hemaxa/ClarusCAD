#include "ViewportWidget.h"
#include "Scene.h"
#include "Segment.h"
#include "BaseTool.h"

#include <QPainter>
#include <QMouseEvent>
#include <cmath>

ViewportWidget::ViewportWidget(QWidget *parent)
    : QWidget(parent), m_scene(nullptr), m_activeTool(nullptr)
{
    setMouseTracking(true); // Для отслеживания движения мыши без зажатой кнопки
}

void ViewportWidget::setScene(Scene* scene)
{
    m_scene = scene;
}

void ViewportWidget::setActiveTool(BaseTool* tool)
{
    m_activeTool = tool;
}

void ViewportWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);

    // 1. Фон
    painter.setBrush(QColor(45, 45, 45));
    painter.drawRect(rect());

    // 2. Сетка
    const int gridSize = 25;
    painter.setPen(QPen(QColor(60, 60, 60), 1.0));
    for (int x = 0; x < width(); x += gridSize) {
        painter.drawLine(x, 0, x, height());
    }
    for (int y = 0; y < height(); y += gridSize) {
        painter.drawLine(0, y, width(), y);
    }

    if (!m_scene) {
        return;
    }

    // 3. Рисуем объекты сцены
    painter.setPen(Qt::white);
    painter.setBrush(Qt::white);
    for (const auto& point : m_scene->getPoints()) {
        painter.drawEllipse(QPointF(point.x(), point.y()), 3, 3);
    }

    // Рисуем отрезки
    for (const auto& segment : m_scene->getSegments()) {
        painter.drawLine(QPointF(segment.getStart().x(), segment.getStart().y()),
                         QPointF(segment.getEnd().x(), segment.getEnd().y()));
    }

    // 4. Отрисовка временной геометрии от активного инструмента
    if (m_activeTool) {
        m_activeTool->onPaint(painter);
    }
}

void ViewportWidget::mousePressEvent(QMouseEvent *event)
{
    if (m_activeTool) {
        m_activeTool->onMousePress(event, m_scene, this);
    }
    update();
}

void ViewportWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (m_activeTool) {
        m_activeTool->onMouseMove(event, m_scene, this);
    }
    update();
}

void ViewportWidget::mouseReleaseEvent(QMouseEvent *event)
{
    if (m_activeTool) {
        m_activeTool->onMouseRelease(event, m_scene, this);
    }
    update();
}
