#include "SegmentCreationTool.h"
#include "SegmentCreationPrimitive.h"

#include <QMouseEvent>
#include <QPainter>
#include <memory>

SegmentCreationTool::SegmentCreationTool(QObject* parent)
    : BaseTool(parent), m_currentState(State::Idle) {}

void SegmentCreationTool::onMousePress(QMouseEvent* event, Scene* scene, ViewportPanelWidget* viewport)
{
    if (event->button() == Qt::LeftButton) {
        if (m_currentState == State::Idle) {
            m_firstPoint.setX(event->position().x());
            m_firstPoint.setY(event->position().y());
            m_currentMousePos = m_firstPoint;
            m_currentState = State::WaitingForSecondPoint;
        }
        else if (m_currentState == State::WaitingForSecondPoint) {
            PointCreationPrimitive secondPoint(event->position().x(), event->position().y());
            auto segment = std::make_unique<SegmentCreationPrimitive>(m_firstPoint, secondPoint);
            emit primitiveCreated(segment.release());

            m_currentState = State::Idle;
        }
    } else if (event->button() == Qt::RightButton) {
        m_currentState = State::Idle;
    }
}

void SegmentCreationTool::onMouseMove(QMouseEvent* event, Scene* scene, ViewportPanelWidget* viewport)
{
    if (m_currentState == State::WaitingForSecondPoint) {
        m_currentMousePos.setX(event->position().x());
        m_currentMousePos.setY(event->position().y());
    }
}

void SegmentCreationTool::onMouseRelease(QMouseEvent* event, Scene* scene, ViewportPanelWidget* viewport)
{
    Q_UNUSED(event);
    Q_UNUSED(scene);
    Q_UNUSED(viewport);
}

void SegmentCreationTool::onPaint(QPainter& painter)
{
    if (m_currentState == State::WaitingForSecondPoint) {
        painter.setPen(QPen(QColor(0, 160, 64, 150), 1.5, Qt::DashLine));
        painter.drawLine(QPointF(m_firstPoint.x(), m_firstPoint.y()),
                         QPointF(m_currentMousePos.x(), m_currentMousePos.y()));
    }
}
