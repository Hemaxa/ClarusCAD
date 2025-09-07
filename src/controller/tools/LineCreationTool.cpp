#include "LineCreationTool.h"
#include "Scene.h"
#include "Segment.h"
#include "ViewportWidget.h" // Нужен для получения MainWindow
#include "MainWindow.h" // Нужен для доступа к слоту handleCreateSegment

#include <QMouseEvent>
#include <QPainter>

LineCreationTool::LineCreationTool(QObject* parent)
    : BaseTool(parent), m_currentState(State::Idle)
{
}

void LineCreationTool::onMousePress(QMouseEvent* event, Scene* scene, ViewportWidget* viewport)
{
    if (event->button() == Qt::LeftButton) {
        if (m_currentState == State::Idle) {
            m_firstPoint.setX(event->position().x());
            m_firstPoint.setY(event->position().y());
            m_currentMousePos = m_firstPoint;
            m_currentState = State::WaitingForSecondPoint;
        } else {
            Point secondPoint(event->position().x(), event->position().y());

            // Вместо прямого добавления в сцену,
            // можно отправить сигнал в MainWindow, чтобы он это сделал.
            // Это более правильная архитектура.
            auto* mainWindow = qobject_cast<MainWindow*>(viewport->window());
            if (mainWindow) {
                // Временное решение: найдем способ вызывать handleCreateSegment
                // Пока что добавляем напрямую
                scene->addSegment(Segment(m_firstPoint, secondPoint));
            }

            m_currentState = State::Idle;
        }
    } else if (event->button() == Qt::RightButton) {
        // Отмена по правому клику
        m_currentState = State::Idle;
    }
}

void LineCreationTool::onMouseMove(QMouseEvent* event, Scene* scene, ViewportWidget* viewport)
{
    if (m_currentState == State::WaitingForSecondPoint) {
        m_currentMousePos.setX(event->position().x());
        m_currentMousePos.setY(event->position().y());
    }
}

void LineCreationTool::onMouseRelease(QMouseEvent* event, Scene* scene, ViewportWidget* viewport)
{
    // Пока не используем, т.к. создаем по второму клику
}

void LineCreationTool::onPaint(QPainter& painter)
{
    if (m_currentState == State::WaitingForSecondPoint) {
        painter.setPen(QPen(QColor(0, 160, 64, 150), 1.5, Qt::DashLine)); // Зеленая пунктирная линия
        painter.drawLine(QPointF(m_firstPoint.x(), m_firstPoint.y()),
                         QPointF(m_currentMousePos.x(), m_currentMousePos.y()));

        painter.setBrush(QColor(0, 160, 64));
        painter.drawEllipse(QPointF(m_firstPoint.x(), m_firstPoint.y()), 4, 4);
    }
}
