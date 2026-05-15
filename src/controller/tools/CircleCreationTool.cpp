#include "CircleCreationTool.h"
#include "CirclePrimitive.h"
#include "ViewportPanelWidget.h"
#include "LineStyleManager.h"
#include "SnapManager.h"

#include <QMouseEvent>
#include <QPainter>
#include <QtMath>

CircleCreationTool::CircleCreationTool(QObject* parent) : BaseCreationTool(parent) {}

void CircleCreationTool::setCreationMode(CircleCreationMode mode)
{
    m_mode = mode;
    reset();
}

void CircleCreationTool::onMousePress(QMouseEvent* event, Scene* scene, ViewportPanelWidget* viewport)
{
    if (event->button() == Qt::LeftButton) {
        QPointF snappedPos = viewport->getSnappedPoint(event->position());
        PointPrimitive clickedPoint(snappedPos.x(), snappedPos.y());
        m_currentMousePos = clickedPoint;

        switch (m_mode) {
        case CircleCreationMode::CenterRadius:
        case CircleCreationMode::CenterDiameter:
            if (m_step == 0) {
                m_p1 = clickedPoint; // Центр
                m_step = 1;
                SnapManager::instance().setBasePoint(snappedPos);
            } else {
                // Второй клик завершает построение
                double dist = QLineF(m_p1.getX(), m_p1.getY(), clickedPoint.getX(), clickedPoint.getY()).length();
                double radius = dist;
                if (m_mode == CircleCreationMode::CenterDiameter) {
                    radius = dist / 2.0; // Если задавали диаметр, то радиус в 2 раза меньше
                }

                if (radius > 0) {
                    auto* circle = new CirclePrimitive(m_p1, radius);
                    circle->setColor(m_currentColor);
                    circle->setLineType(m_currentLineType);
                    emit circleDataReady(circle);
                }
                reset();
            }
            break;

        case CircleCreationMode::TwoPoints:
            if (m_step == 0) {
                m_p1 = clickedPoint; // Первая точка диаметра
                m_step = 1;
                SnapManager::instance().setBasePoint(snappedPos);
            } else {
                // Второй клик - вторая точка диаметра
                m_p2 = clickedPoint;
                PointPrimitive center((m_p1.getX() + m_p2.getX()) / 2.0, (m_p1.getY() + m_p2.getY()) / 2.0);
                double radius = QLineF(m_p1.getX(), m_p1.getY(), m_p2.getX(), m_p2.getY()).length() / 2.0;

                if (radius > 0) {
                    auto* circle = new CirclePrimitive(center, radius);
                    circle->setColor(m_currentColor);
                    circle->setLineType(m_currentLineType);
                    emit circleDataReady(circle);
                }
                reset();
            }
            break;

        case CircleCreationMode::ThreePoints:
            if (m_step == 0) {
                m_p1 = clickedPoint;
                m_step = 1;
                SnapManager::instance().setBasePoint(snappedPos);
            } else if (m_step == 1) {
                m_p2 = clickedPoint;
                m_step = 2;
            } else {
                m_p3 = clickedPoint;
                PointPrimitive center;
                double radius = 0;
                if (getCircleFrom3Points(m_p1, m_p2, m_p3, center, radius)) {
                    auto* circle = new CirclePrimitive(center, radius);
                    circle->setColor(m_currentColor);
                    circle->setLineType(m_currentLineType);
                    emit circleDataReady(circle);
                }
                reset();
            }
            break;
        }
    }
    else if (event->button() == Qt::RightButton) {
        reset();
    }
}

void CircleCreationTool::onMouseMove(QMouseEvent* event, Scene* scene, ViewportPanelWidget* viewport)
{
    if (m_step > 0) {
        QPointF snappedPos = viewport->getSnappedPoint(event->position());
        m_currentMousePos.setX(snappedPos.x());
        m_currentMousePos.setY(snappedPos.y());
    }
}

void CircleCreationTool::onMouseRelease(QMouseEvent* event, Scene* scene, ViewportPanelWidget* viewport) {}

void CircleCreationTool::reset()
{
    m_step = 0;
    SnapManager::instance().clearBasePoint();
}

void CircleCreationTool::setColor(const QColor& color) { m_currentColor = color; }
void CircleCreationTool::setLineType(LineType type) { m_currentLineType = type; }
QColor CircleCreationTool::getColor() const { return m_currentColor; }

void CircleCreationTool::onPaint(QPainter& painter)
{
    if (m_step == 0) return;
    const double currentScale = std::max(1e-6, std::hypot(painter.transform().m11(), painter.transform().m12()));

    QColor previewColor = m_currentColor;
    previewColor.setAlpha(180);

    QPointF current(m_currentMousePos.getX(), m_currentMousePos.getY());
    QPointF p1Pt(m_p1.getX(), m_p1.getY());

    // Стандартный стиль для ВСЕХ служебных линий
    QPen guidePen(Qt::white);
    guidePen.setStyle(Qt::DashLine);
    guidePen.setWidthF(1.0 / currentScale);
    guidePen.setCosmetic(true);

    // Временные переменные для расчета параметров предпросмотра
    QPointF center;
    double radius = 0.0;
    bool isValid = false;

    if (m_mode == CircleCreationMode::CenterRadius) {
        center = p1Pt;
        radius = QLineF(center, current).length();
        isValid = true;
        
        // Радиус-линия (белая пунктирная)
        painter.setPen(guidePen);
        painter.drawLine(center, current);
    }
    else if (m_mode == CircleCreationMode::CenterDiameter) {
        center = p1Pt;
        radius = QLineF(center, current).length() / 2.0;
        isValid = true;
        
        // Диаметр-линия (белая пунктирная)
        painter.setPen(guidePen);
        QPointF opposite = center - (current - center);
        painter.drawLine(opposite, current);
    } 
    else if (m_mode == CircleCreationMode::TwoPoints) {
        center = (p1Pt + current) / 2.0;
        radius = QLineF(p1Pt, current).length() / 2.0;
        isValid = true;
        
        // Линия диаметра (белая пунктирная)
        painter.setPen(guidePen);
        painter.drawLine(p1Pt, current);
    }
    else if (m_mode == CircleCreationMode::ThreePoints) {
        if (m_step == 1) {
            // Линия между первой и текущей (белая пунктирная)
            painter.setPen(guidePen);
            painter.drawLine(p1Pt, current);
        } else if (m_step == 2) {
            QPointF p2Pt(m_p2.getX(), m_p2.getY());
            // Линии между точками (белые пунктирные)
            painter.setPen(guidePen);
            painter.drawLine(p1Pt, p2Pt);
            painter.drawLine(p2Pt, current);
            
            PointPrimitive c;
            if (getCircleFrom3Points(m_p1, m_p2, m_currentMousePos, c, radius)) {
                center = QPointF(c.getX(), c.getY());
                isValid = true;
            }
        }
    }

    // Предпросмотр окружности (в цвете пользователя)
    if (isValid && radius > 0) {
        LineStyleManager::instance().drawEllipse(
            painter,
            center,
            radius, radius,
            static_cast<int>(m_currentLineType),
            previewColor
            );
    }
    
    // ЖИРНЫЕ МАРКЕРЫ ТОЧЕК ПОЛЬЗОВАТЕЛЯ
    QPen markerPen(Qt::white, 2.0 / currentScale);
    markerPen.setCosmetic(true);
    painter.setPen(markerPen);
    painter.setBrush(m_currentColor);
    const double markerSize = 6.0 / currentScale;
    
    // Первая точка
    painter.drawEllipse(p1Pt, markerSize, markerSize);
    
    // Вторая точка (если есть)
    if (m_step >= 2 && m_mode == CircleCreationMode::ThreePoints) {
        QPointF p2Pt(m_p2.getX(), m_p2.getY());
        painter.drawEllipse(p2Pt, markerSize, markerSize);
    }
}

// Математика: Окружность по 3 точкам
bool CircleCreationTool::getCircleFrom3Points(const PointPrimitive& p1, const PointPrimitive& p2, const PointPrimitive& p3, PointPrimitive& center, double& radius)
{
    double x1 = p1.getX(), y1 = p1.getY();
    double x2 = p2.getX(), y2 = p2.getY();
    double x3 = p3.getX(), y3 = p3.getY();

    double D = 2 * (x1 * (y2 - y3) + x2 * (y3 - y1) + x3 * (y1 - y2));

    if (std::abs(D) < 1e-9) return false; // Точки на одной прямой

    double centerX = ((x1 * x1 + y1 * y1) * (y2 - y3) + (x2 * x2 + y2 * y2) * (y3 - y1) + (x3 * x3 + y3 * y3) * (y1 - y2)) / D;
    double centerY = ((x1 * x1 + y1 * y1) * (x3 - x2) + (x2 * x2 + y2 * y2) * (x1 - x3) + (x3 * x3 + y3 * y3) * (x2 - x1)) / D;

    center.setX(centerX);
    center.setY(centerY);

    radius = std::sqrt(std::pow(centerX - x1, 2) + std::pow(centerY - y1, 2));
    return true;
}
