#include "SegmentCreationTool.h"

#include <QMouseEvent>
#include <QPainter>

//вызывается конструктор базового класса и устанавливается начальное состояние в Idle (покой)
SegmentCreationTool::SegmentCreationTool(QObject* parent) : BaseCreationTool(parent), m_currentState(State::Idle) {}

void SegmentCreationTool::onMousePress(QMouseEvent* event, Scene* scene, ViewportPanelWidget* viewport)
{
    //если нажата ЛКМ
    if (event->button() == Qt::LeftButton) {
        //если состояние покоя (первый клик)
        if (m_currentState == State::Idle) {
            //координаты первой точки сохраняются в примитив
            m_firstPoint.setX(event->position().x());
            m_firstPoint.setY(event->position().y());

            //позиция мыши сохраняется для отрисовки
            m_currentMousePos = m_firstPoint;

            //состояние изменяется в "Ожидание второй точки"
            m_currentState = State::WaitingForSecondPoint;
        }
        //иначе (второй клик)
        else if (m_currentState == State::WaitingForSecondPoint) {
            //координаты второй точки сохраняются в новый примитив
            PointPrimitive secondPoint(event->position().x(), event->position().y());

            //посылается сигнал о готовности передачи
            emit segmentDataReady(m_firstPoint, secondPoint);

            //состояние изменяется в "Покой"
            m_currentState = State::Idle;
        }
    }
    //если нажата ПКМ
    else if (event->button() == Qt::RightButton) {
        //отмена операции и переход в состояние "Покой"
        m_currentState = State::Idle;
    }
}

void SegmentCreationTool::onMouseMove(QMouseEvent* event, Scene* scene, ViewportPanelWidget* viewport)
{
    //если запущен процесс создания отрезка
    if (m_currentState == State::WaitingForSecondPoint) {
        //координаты мыши обновляются
        m_currentMousePos.setX(event->position().x());
        m_currentMousePos.setY(event->position().y());
    }
}

void SegmentCreationTool::onMouseRelease(QMouseEvent* event, Scene* scene, ViewportPanelWidget* viewport)
{
    //в логике инструмента "Отрезок" отпускание кнопки не играет роли
    Q_UNUSED(event);
    Q_UNUSED(scene);
    Q_UNUSED(viewport);
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
