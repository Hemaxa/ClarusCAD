#include "SegmentCreationTool.h"
#include "ViewportPanelWidget.h"
#include "LineStyleManager.h"

#include <QMouseEvent>
#include <QPainter>

//вызывается конструктор базового класса и устанавливается начальное состояние в Idle (покой)
SegmentCreationTool::SegmentCreationTool(QObject* parent) : BaseCreationTool(parent), m_currentState(State::Idle) {}

void SegmentCreationTool::onMousePress(QMouseEvent* event, Scene* scene, ViewportPanelWidget* viewport)
{
    //создание нового примитива
    if (event->button() == Qt::LeftButton) {
        //определение координат, в зависимости от активации опции "Привязка к сетке"
        QPointF snappedPos = viewport->getSnappedPoint(event->position());

        //первый клик
        if (m_currentState == State::Idle) {
            m_firstPoint.setX(snappedPos.x());
            m_firstPoint.setY(snappedPos.y());
            m_currentMousePos = m_firstPoint;
            m_currentState = State::WaitingForSecondPoint;
        }
        //второй клик
        else if (m_currentState == State::WaitingForSecondPoint) {
            PointPrimitive secondPoint(snappedPos.x(), snappedPos.y());
            emit segmentDataReady(nullptr, m_firstPoint, secondPoint, m_currentColor, m_currentLineType);
            m_currentState = State::Idle;
        }
    }
    //отмена создания
    else if (event->button() == Qt::RightButton) {
        m_currentState = State::Idle;
    }
}

void SegmentCreationTool::onMouseMove(QMouseEvent* event, Scene* scene, ViewportPanelWidget* viewport)
{
    //приклеивание к сетке, если включена опция "Привязка к сетке"
    if (m_currentState == State::WaitingForSecondPoint) {
        QPointF snappedPos = viewport->getSnappedPoint(event->position());
        m_currentMousePos.setX(snappedPos.x());
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

        //формируем полупрозрачный цвет для предпросмотра
        QColor previewColor = m_currentColor;
        previewColor.setAlpha(150);

        QPointF firstPt(m_firstPoint.getX(), m_firstPoint.getY());
        QPointF currentPt(m_currentMousePos.getX(), m_currentMousePos.getY());

        //используем менеджер стилей для отрисовки того типа линии, который выбран
        LineStyleManager::instance().drawLine(
            painter,
            firstPt,
            currentPt,
            static_cast<int>(m_currentLineType),
            previewColor
            );
        
        // ЖИРНЫЙ МАРКЕР ПЕРВОЙ ТОЧКИ
        painter.setPen(QPen(Qt::white, 2.0));
        painter.setBrush(m_currentColor);
        painter.drawEllipse(firstPt, 6, 6);
    }
}

void SegmentCreationTool::setColor(const QColor& color) { m_currentColor = color; }
void SegmentCreationTool::setLineType(LineType type) { m_currentLineType = type; }

QColor SegmentCreationTool::getColor() const { return m_currentColor; }
