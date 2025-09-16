#include "SegmentCreationTool.h"
#include "ViewportPanelWidget.h"

#include <QMouseEvent>
#include <QPainter>

//вызывается конструктор базового класса и устанавливается начальное состояние в Idle (покой)
SegmentCreationTool::SegmentCreationTool(QObject* parent) : BaseCreationTool(parent), m_currentState(State::Idle) {}

void SegmentCreationTool::onMousePress(QMouseEvent* event, Scene* scene, ViewportPanelWidget* viewport)
{
    if (event->button() == Qt::LeftButton) {
        QPointF snappedPos = viewport->snapToGrid(event->position()); // <-- Привязываем позицию

        if (m_currentState == State::Idle) {
            m_firstPoint.setX(snappedPos.x()); // <-- Используем привязанную позицию
            m_firstPoint.setY(snappedPos.y());
            m_currentMousePos = m_firstPoint;
            m_currentState = State::WaitingForSecondPoint;
        }
        else if (m_currentState == State::WaitingForSecondPoint) {
            PointPrimitive secondPoint(snappedPos.x(), snappedPos.y()); // <-- Используем привязанную позицию
            emit segmentDataReady(m_firstPoint, secondPoint);
            m_currentState = State::Idle;
        }
    }
    else if (event->button() == Qt::RightButton) {
        m_currentState = State::Idle;
    }
}

void SegmentCreationTool::onMouseMove(QMouseEvent* event, Scene* scene, ViewportPanelWidget* viewport)
{
    if (m_currentState == State::WaitingForSecondPoint) {
        QPointF snappedPos = viewport->snapToGrid(event->position()); // <-- Привязываем позицию
        m_currentMousePos.setX(snappedPos.x()); // <-- Используем привязанную позицию
        m_currentMousePos.setY(snappedPos.y());
    }
}

void SegmentCreationTool::onMouseRelease(QMouseEvent* event, Scene* scene, ViewportPanelWidget* viewport)
{
    //в логике инструмента "Отрезок" отпускание кнопки не играет роли
    Q_UNUSED(event);
    Q_UNUSED(scene);
    Q_UNUSED(viewport);
}

void SegmentCreationTool::reset()
{
    //состояние сбрасывается в "Покой"
    m_currentState = State::Idle;
}

void SegmentCreationTool::onPaint(QPainter& painter)
{
    //если запущен процесс создания отрезка
    if (m_currentState == State::WaitingForSecondPoint) {
        //рисуется вспомогательная линия
        painter.setPen(QPen(QColor(0, 160, 64, 150), 1.5, Qt::DashLine));
        painter.drawLine(QPointF(m_firstPoint.getX(), m_firstPoint.getY()), QPointF(m_currentMousePos.getX(), m_currentMousePos.getY()));
    }
}
