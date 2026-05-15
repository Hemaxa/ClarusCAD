#include "SplineCreationTool.h"
#include "SplinePrimitive.h"
#include "ViewportPanelWidget.h"
#include "Scene.h"

#include <QMouseEvent>
#include <QKeyEvent>
#include <QPainter>

SplineCreationTool::SplineCreationTool(QObject* parent)
    : BaseCreationTool(parent)
{
}

void SplineCreationTool::onMousePress(QMouseEvent* event, Scene* scene, ViewportPanelWidget* viewport)
{
    Q_UNUSED(scene);
    
    if (event->button() == Qt::LeftButton) {
        // Добавляем контрольную точку
        QPointF worldPos = viewport->getSnappedPoint(event->position());
        m_controlPoints.append(worldPos);
        viewport->update();
    }
    else if (event->button() == Qt::RightButton) {
        // Правый клик — завершаем построение
        finishSpline();
    }
}

void SplineCreationTool::onMouseMove(QMouseEvent* event, Scene* scene, ViewportPanelWidget* viewport)
{
    Q_UNUSED(scene);
    
    QPointF worldPos = viewport->getSnappedPoint(event->position());
    m_currentMousePos = worldPos;
    
    viewport->update();
}

void SplineCreationTool::onMouseRelease(QMouseEvent* event, Scene* scene, ViewportPanelWidget* viewport)
{
    Q_UNUSED(event);
    Q_UNUSED(scene);
    Q_UNUSED(viewport);
}

void SplineCreationTool::onKeyPress(QKeyEvent* event, Scene* scene, ViewportPanelWidget* viewport)
{
    Q_UNUSED(scene);
    Q_UNUSED(viewport);
    
    if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) {
        finishSpline();
        event->accept();
    }
    else if (event->key() == Qt::Key_Escape) {
        if (!m_controlPoints.isEmpty()) {
            reset();
            if (viewport) viewport->update();
            event->accept();
        } else {
            event->ignore();
        }
    }
    else if (event->key() == Qt::Key_Backspace && !m_controlPoints.isEmpty()) {
        // Удалить последнюю точку
        m_controlPoints.removeLast();
        if (viewport) viewport->update();
        event->accept();
    }
}

void SplineCreationTool::reset()
{
    m_controlPoints.clear();
    m_currentMousePos = QPointF();
    m_closed = false;
}

void SplineCreationTool::finishSpline()
{
    if (m_controlPoints.size() < 2) {
        reset();
        return;
    }
    
    auto* spline = new SplinePrimitive(m_controlPoints);
    spline->setColor(m_currentColor);
    spline->setLineType(static_cast<int>(m_currentLineType));
    spline->setClosed(m_closed);
    
    emit splineDataReady(spline);
    reset();
}

void SplineCreationTool::onPaint(QPainter& painter)
{
    if (m_controlPoints.isEmpty()) return;
    const double currentScale = std::max(1e-6, std::hypot(painter.transform().m11(), painter.transform().m12()));
    
    // Стандартный стиль для служебных линий
    QPen guidePen(Qt::white);
    guidePen.setStyle(Qt::DashLine);
    guidePen.setWidthF(1.0 / currentScale);
    guidePen.setCosmetic(true);
    
    // Рисуем линии между контрольными точками (белые пунктирные)
    painter.setPen(guidePen);
    for (int i = 0; i < m_controlPoints.size() - 1; ++i) {
        painter.drawLine(m_controlPoints[i], m_controlPoints[i + 1]);
    }
    if (!m_controlPoints.isEmpty()) {
        painter.drawLine(m_controlPoints.last(), m_currentMousePos);
    }
    
    // Рисуем временный сплайн с текущей позицией мыши (в цвете пользователя)
    QVector<QPointF> tempPoints = m_controlPoints;
    tempPoints.append(m_currentMousePos);
    
    SplinePrimitive tempSpline(tempPoints);
    
    QColor previewColor = m_currentColor;
    previewColor.setAlpha(180);
    QPen previewPen(previewColor);
    previewPen.setWidthF(1.5 / currentScale);
    previewPen.setCosmetic(true);
    painter.setPen(previewPen);
    painter.setBrush(Qt::NoBrush);
    
    tempSpline.draw(painter, false);
    
    // ЖИРНЫЕ МАРКЕРЫ КОНТРОЛЬНЫХ ТОЧЕК
    QPen markerPen(Qt::white, 2.0 / currentScale);
    markerPen.setCosmetic(true);
    painter.setPen(markerPen);
    painter.setBrush(m_currentColor);
    
    for (const auto& pt : m_controlPoints) {
        painter.drawEllipse(pt, 6.0 / currentScale, 6.0 / currentScale);
    }
    
    // Текущая позиция мыши
    painter.setBrush(QColor(0, 255, 127, 200));
    painter.drawEllipse(m_currentMousePos, 5.0 / currentScale, 5.0 / currentScale);
}

void SplineCreationTool::setColor(const QColor& color)
{
    m_currentColor = color;
}

void SplineCreationTool::setLineType(LineType type)
{
    m_currentLineType = type;
}

QColor SplineCreationTool::getColor() const
{
    return m_currentColor;
}
