#include "ViewportPanelWidget.h"
#include "Scene.h"
#include "BaseTool.h"

#include <QPainter>
#include <QMouseEvent>

ViewportPanelWidget::ViewportPanelWidget(QWidget* parent) : QWidget(parent)
{
    setMouseTracking(true);
}

void ViewportPanelWidget::setScene(Scene* scene) { m_scene = scene; }
void ViewportPanelWidget::setActiveTool(BaseTool* tool) { m_activeTool = tool; }

void ViewportPanelWidget::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    painter.setBrush(QColor(45, 45, 45));
    painter.drawRect(rect());

    if (!m_scene) return;

    painter.setPen(Qt::white);
    for (const auto& primitive : m_scene->getPrimitives()) {
        if (primitive->getType() == PrimitiveType::Segment) {
            auto* segment = static_cast<SegmentCreationPrimitive*>(primitive.get());
            painter.drawLine(
                QPointF(segment->getStart().x(), segment->getStart().y()),
                QPointF(segment->getEnd().x(), segment->getEnd().y())
                );
        }
    }

    if (m_activeTool) {
        m_activeTool->onPaint(painter);
    }
}

void ViewportPanelWidget::mousePressEvent(QMouseEvent* event)
{
    if (m_activeTool) {
        m_activeTool->onMousePress(event, m_scene, this);
    }
    update();
}

void ViewportPanelWidget::mouseMoveEvent(QMouseEvent* event)
{
    if (m_activeTool) {
        m_activeTool->onMouseMove(event, m_scene, this);
    }
    update();
}

void ViewportPanelWidget::mouseReleaseEvent(QMouseEvent* event)
{
    if (m_activeTool) {
        m_activeTool->onMouseRelease(event, m_scene, this);
    }
    update();
}
