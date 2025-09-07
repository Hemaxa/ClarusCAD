#include "ViewportWidget.h"
#include "Scene.h"

#include <QPainter>

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
    painter.setBrush(Qt::white);
    for (const auto& point : m_scene->getPoints()) {
        painter.drawEllipse(QPointF(point.x(), point.y()), 3, 3);
    }
}
