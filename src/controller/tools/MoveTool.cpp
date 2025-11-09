#include "MoveTool.h"
#include "ViewportPanelWidget.h"

#include <QWidget>
#include <QCursor>
#include <QApplication>

MoveTool::MoveTool(QObject* parent) : BaseCreationTool(parent), m_isActive(false)
{
    m_panTimer = new QTimer(this);
    connect(m_panTimer, &QTimer::timeout, this, &MoveTool::onPanTimerTimeout);
}

void MoveTool::reset()
{
    deactivate();
}

void MoveTool::activate(ViewportPanelWidget* viewport)
{
    if (m_isActive) return;

    m_viewport = viewport;
    m_isActive = true;
    m_currentMousePos = m_viewport->getCanvas()->mapFromGlobal(QCursor::pos());
    m_panTimer->start(16);
}

void MoveTool::deactivate()
{
    if (!m_isActive) return;

    m_isActive = false;
    m_panTimer->stop();
    m_viewport = nullptr;
}

bool MoveTool::isActive() const
{
    return m_isActive;
}

void MoveTool::updateMousePosition(const QPoint& screenPos)
{
    if (m_isActive) {
        m_currentMousePos = screenPos;
    }
}

void MoveTool::onPanTimerTimeout()
{
    if (!m_viewport) return;

    QSize canvasSize = m_viewport->getCanvas()->size();
    QPointF worldDelta(0.0, 0.0); //БЫЛО: panDelta

    // логика панорамирования в МИРОВЫХ координатах (Y-up)
    // +X -> мир вправо (сцена влево)
    // -X -> мир влево (сцена вправо)
    // +Y -> мир вверх (сцена вниз)
    // -Y -> мир вниз (сцена вверх)

    //ось X
    //мышь у левого края -> панорамируем мир вправо (+X)
    if (m_currentMousePos.x() < m_borderThreshold) {
        double distance = m_borderThreshold - m_currentMousePos.x();
        double speedFactor = std::max(0.0, std::min(1.0, distance / m_borderThreshold));
        worldDelta.setX(m_maxPanSpeed * speedFactor); // Pan World +X
    }
    //мышь у правого края -> панорамируем мир влево (-X)
    else if (m_currentMousePos.x() > canvasSize.width() - m_borderThreshold) {
        double distance = m_currentMousePos.x() - (canvasSize.width() - m_borderThreshold);
        double speedFactor = std::max(0.0, std::min(1.0, distance / m_borderThreshold));
        worldDelta.setX(-m_maxPanSpeed * speedFactor); // Pan World -X
    }

    //ось Y (Мир Y-up: +Y = вверх)
    //мышь у верхнего края -> панорамируем мир вниз (-Y)
    if (m_currentMousePos.y() < m_borderThreshold) {
        double distance = m_borderThreshold - m_currentMousePos.y();
        double speedFactor = std::max(0.0, std::min(1.0, distance / m_borderThreshold));
        worldDelta.setY(-m_maxPanSpeed * speedFactor); // ИЗМЕНЕН ЗНАК
    }
    //мышь у нижнего края -> панорамируем мир вверх (+Y)
    else if (m_currentMousePos.y() > canvasSize.height() - m_borderThreshold) {
        double distance = m_currentMousePos.y() - (canvasSize.height() - m_borderThreshold);
        double speedFactor = std::max(0.0, std::min(1.0, distance / m_borderThreshold));
        worldDelta.setY(m_maxPanSpeed * speedFactor); // ИЗМЕНЕН ЗНАК
    }

    if (!worldDelta.isNull()) {
        // m_viewport->pan(panDelta); // <-- СТАРОЕ
        m_viewport->panWorld(worldDelta); // <-- НОВОЕ
    }
}

void MoveTool::onMousePress(QMouseEvent* event, Scene* scene, ViewportPanelWidget* viewport) { Q_UNUSED(event); Q_UNUSED(scene); Q_UNUSED(viewport); }
void MoveTool::onMouseMove(QMouseEvent* event, Scene* scene, ViewportPanelWidget* viewport) { Q_UNUSED(event); Q_UNUSED(scene); Q_UNUSED(viewport); }
void MoveTool::onMouseRelease(QMouseEvent* event, Scene* scene, ViewportPanelWidget* viewport) { Q_UNUSED(event); Q_UNUSED(scene); Q_UNUSED(viewport); }
