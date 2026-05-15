#include "PolygonCreationTool.h"
#include "PolygonPrimitive.h"
#include "ViewportPanelWidget.h"
#include "Scene.h"

#include <QMouseEvent>
#include <QPainter>
#include <cmath>

PolygonCreationTool::PolygonCreationTool(QObject* parent)
    : BaseCreationTool(parent)
{
}

void PolygonCreationTool::onMousePress(QMouseEvent* event, Scene* scene, ViewportPanelWidget* viewport)
{
    Q_UNUSED(scene);
    
    if (event->button() != Qt::LeftButton) return;
    
    QPointF worldPos = viewport->getSnappedPoint(event->position());
    m_currentMousePos = PointPrimitive(worldPos.x(), worldPos.y());
    
    if (m_step == 0) {
        // Первый клик — центр
        m_center = PointPrimitive(worldPos.x(), worldPos.y());
        m_step = 1;
    } else if (m_step == 1) {
        // Второй клик — радиус
        double radius = QLineF(QPointF(m_center.getX(), m_center.getY()), worldPos).length();
        
        // Вычисляем угол поворота (первая вершина направлена к курсору)
        double rotation = std::atan2(worldPos.y() - m_center.getY(), 
                                      worldPos.x() - m_center.getX()) * 180.0 / M_PI;
        
        PolygonType pType = (m_polygonMode == PolygonCreationMode::Inscribed) ? 
                            PolygonType::Inscribed : PolygonType::Circumscribed;
        
        auto* polygon = new PolygonPrimitive(m_center, radius, m_sides, pType, rotation);
        polygon->setColor(m_currentColor);
        polygon->setLineType(static_cast<int>(m_currentLineType));
        
        emit polygonDataReady(polygon);
        reset();
    }
}

void PolygonCreationTool::onMouseMove(QMouseEvent* event, Scene* scene, ViewportPanelWidget* viewport)
{
    Q_UNUSED(scene);
    
    QPointF worldPos = viewport->getSnappedPoint(event->position());
    m_currentMousePos = PointPrimitive(worldPos.x(), worldPos.y());
    
    viewport->update();
}

void PolygonCreationTool::onMouseRelease(QMouseEvent* event, Scene* scene, ViewportPanelWidget* viewport)
{
    Q_UNUSED(event);
    Q_UNUSED(scene);
    Q_UNUSED(viewport);
}

void PolygonCreationTool::reset()
{
    m_step = 0;
    m_center = PointPrimitive();
    m_currentMousePos = PointPrimitive();
}

void PolygonCreationTool::onPaint(QPainter& painter)
{
    if (m_step != 1) return;
    const double currentScale = std::max(1e-6, std::hypot(painter.transform().m11(), painter.transform().m12()));
    
    // Стандартный стиль для служебных линий
    QPen guidePen(Qt::white);
    guidePen.setStyle(Qt::DashLine);
    guidePen.setWidthF(1.0 / currentScale);
    guidePen.setCosmetic(true);
    
    QPointF centerPt(m_center.getX(), m_center.getY());
    QPointF mousePt(m_currentMousePos.getX(), m_currentMousePos.getY());
    
    double radius = QLineF(centerPt, mousePt).length();
    double rotation = std::atan2(mousePt.y() - centerPt.y(), 
                                  mousePt.x() - centerPt.x()) * 180.0 / M_PI;
    
    // Создаем временный многоугольник для предпросмотра
    PolygonType pType = (m_polygonMode == PolygonCreationMode::Inscribed) ? 
                        PolygonType::Inscribed : PolygonType::Circumscribed;
    PolygonPrimitive tempPolygon(m_center, radius, m_sides, pType, rotation);
    
    QVector<QPointF> vertices = tempPolygon.getVertices();
    
    // Предпросмотр многоугольника (в цвете пользователя)
    QColor previewColor = m_currentColor;
    previewColor.setAlpha(180);
    QPen previewPen(previewColor);
    previewPen.setWidthF(1.5 / currentScale);
    previewPen.setCosmetic(true);
    painter.setPen(previewPen);
    painter.setBrush(Qt::NoBrush);
    
    if (!vertices.isEmpty()) {
        QPolygonF polygon(vertices);
        polygon.append(vertices.first());
        painter.drawPolygon(polygon);
    }
    
    // Радиус-линия (белая пунктирная)
    painter.setPen(guidePen);
    painter.drawLine(centerPt, mousePt);
    
    // ЖИРНЫЙ МАРКЕР ЦЕНТРА
    QPen markerPen(Qt::white, 2.0 / currentScale);
    markerPen.setCosmetic(true);
    painter.setPen(markerPen);
    painter.setBrush(m_currentColor);
    painter.drawEllipse(centerPt, 6.0 / currentScale, 6.0 / currentScale);
}

void PolygonCreationTool::setColor(const QColor& color)
{
    m_currentColor = color;
}

void PolygonCreationTool::setLineType(LineType type)
{
    m_currentLineType = type;
}

QColor PolygonCreationTool::getColor() const
{
    return m_currentColor;
}

void PolygonCreationTool::setSides(int sides)
{
    m_sides = (sides >= 3) ? sides : 3;
}

int PolygonCreationTool::getSides() const
{
    return m_sides;
}

void PolygonCreationTool::setPolygonType(PolygonCreationMode mode)
{
    m_polygonMode = mode;
}

PolygonCreationMode PolygonCreationTool::getPolygonType() const
{
    return m_polygonMode;
}
