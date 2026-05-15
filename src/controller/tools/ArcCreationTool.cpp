#include "ArcCreationTool.h"
#include "ArcPrimitive.h"
#include "ViewportPanelWidget.h"
#include "LineStyleManager.h"

#include <QMouseEvent>
#include <QtMath>
#include <QPainter>

ArcCreationTool::ArcCreationTool(QObject* parent) : BaseCreationTool(parent) {}

void ArcCreationTool::setCreationMode(ArcCreationMode mode) {
    m_mode = mode;
    reset();
}

void ArcCreationTool::onMousePress(QMouseEvent* event, Scene* scene, ViewportPanelWidget* viewport)
{
    if (event->button() == Qt::LeftButton) {
        QPointF snapped = viewport->getSnappedPoint(event->position());
        PointPrimitive pt(snapped.x(), snapped.y());

        if (m_mode == ArcCreationMode::CenterStartEnd) {
            // Старая логика
            if (m_step == 0) {
                m_p1 = pt; // Center
                m_currentPos = pt;
                m_step = 1;
            } else if (m_step == 1) {
                m_p2 = pt; // Start Point
                m_step = 2;
            } else if (m_step == 2) {
                // End Point
                m_p3 = pt;
                double radius = QLineF(m_p1.getX(), m_p1.getY(), m_p2.getX(), m_p2.getY()).length();
                double startAngle = QLineF(m_p1.getX(), m_p1.getY(), m_p2.getX(), m_p2.getY()).angle();
                double endAngle = QLineF(m_p1.getX(), m_p1.getY(), pt.getX(), pt.getY()).angle();

                double span = endAngle - startAngle;
                if (span < 0) span += 360.0; // Counter-clockwise

                auto* arc = new ArcPrimitive(m_p1, radius, startAngle, span);
                arc->setColor(m_currentColor);
                arc->setLineType(m_currentLineType);

                emit arcDataReady(arc);
                reset();
            }
        }
        else if (m_mode == ArcCreationMode::ThreePoints) {
            if (m_step == 0) {
                m_p1 = pt; // Start
                m_currentPos = pt;
                m_step = 1;
            } else if (m_step == 1) {
                m_p2 = pt; // Mid
                m_step = 2;
            } else if (m_step == 2) {
                // End point (pt)
                m_p3 = pt;
                // Строим дугу по 3 точкам: m_p1, m_p2, pt

                // 1. Находим центр окружности по 3 точкам (алгоритм как у круга)
                double x1 = m_p1.getX(), y1 = m_p1.getY();
                double x2 = m_p2.getX(), y2 = m_p2.getY();
                double x3 = pt.getX(), y3 = pt.getY();

                double D = 2 * (x1 * (y2 - y3) + x2 * (y3 - y1) + x3 * (y1 - y2));

                if (std::abs(D) > 1e-9) {
                    double cx = ((x1*x1 + y1*y1)*(y2-y3) + (x2*x2 + y2*y2)*(y3-y1) + (x3*x3 + y3*y3)*(y1-y2)) / D;
                    double cy = ((x1*x1 + y1*y1)*(x3-x2) + (x2*x2 + y2*y2)*(x1-x3) + (x3*x3 + y3*y3)*(x2-x1)) / D;

                    PointPrimitive center(cx, cy);
                    double radius = std::sqrt(std::pow(cx - x1, 2) + std::pow(cy - y1, 2));

                    // 2. Считаем углы для Start (p1), Mid (p2), End (pt)
                    double a1 = QLineF(cx, cy, x1, y1).angle();
                    double a2 = QLineF(cx, cy, x2, y2).angle();
                    double a3 = QLineF(cx, cy, x3, y3).angle();

                    // 3. Определяем направление и span.
                    // Дуга должна проходить через p2.
                    // Span = a3 - a1.
                    // Проверяем, попадает ли a2 в интервал [a1, a1+span]

                    double span = a3 - a1;
                    // Нормализуем span в -360..360?
                    // Нет, нам нужно понять направление (CW или CCW).

                    // Простейший способ:
                    // Если идти CCW от a1 до a3, лежит ли a2 внутри?
                    double a2_rel = a2 - a1; if (a2_rel < 0) a2_rel += 360.0;
                    double a3_rel = a3 - a1; if (a3_rel < 0) a3_rel += 360.0;

                    double finalStart = a1;
                    double finalSpan = 0;

                    if (a2_rel < a3_rel) {
                        // Порядок: Start -> Mid -> End (CCW)
                        finalSpan = a3_rel;
                    } else {
                        // Порядок: Start -> End -> Mid (CCW), значит Mid был "с другой стороны"
                        // Значит нужно идти в другую сторону (CW) или span отрицательный
                        // Либо span = a3_rel - 360.0
                        finalSpan = a3_rel - 360.0;
                    }

                    auto* arc = new ArcPrimitive(center, radius, finalStart, finalSpan);
                    arc->setColor(m_currentColor);
                    arc->setLineType(m_currentLineType);
                    emit arcDataReady(arc);
                }
                reset();
            }
        }
        viewport->update();
    } else if (event->button() == Qt::RightButton) {
        reset();
        viewport->update();
    }
}

void ArcCreationTool::onMouseMove(QMouseEvent* event, Scene* scene, ViewportPanelWidget* viewport) {
    if (m_step > 0) {
        QPointF snapped = viewport->getSnappedPoint(event->position());
        m_currentPos = PointPrimitive(snapped.x(), snapped.y());
        viewport->update();
    }
}

void ArcCreationTool::onMouseRelease(QMouseEvent*, Scene*, ViewportPanelWidget*) {}
void ArcCreationTool::reset() { m_step = 0; }

void ArcCreationTool::onPaint(QPainter& painter) {
    if (m_step == 0) return;
    const double currentScale = std::max(1e-6, std::hypot(painter.transform().m11(), painter.transform().m12()));

    // Стандартный стиль для служебных линий (белый пунктир)
    QPen guidePen(Qt::white);
    guidePen.setStyle(Qt::DashLine);
    guidePen.setWidthF(1.0 / currentScale);
    guidePen.setCosmetic(true);
    
    QPointF p1Pt(m_p1.getX(), m_p1.getY());
    QPointF currentPt(m_currentPos.getX(), m_currentPos.getY());
    
    if (m_mode == ArcCreationMode::CenterStartEnd) {
        if (m_step >= 1) {
            // Линия от центра к текущей позиции
            painter.setPen(guidePen);
            painter.drawLine(p1Pt, currentPt);
        }
        if (m_step >= 2) {
            QPointF p2Pt(m_p2.getX(), m_p2.getY());
            // Линия от центра к начальной точке
            painter.setPen(guidePen);
            painter.drawLine(p1Pt, p2Pt);
            
            // Предпросмотр дуги
            double radius = QLineF(p1Pt, p2Pt).length();
            double startAngle = QLineF(p1Pt, p2Pt).angle();
            double endAngle = QLineF(p1Pt, currentPt).angle();
            double span = endAngle - startAngle;
            if (span < 0) span += 360.0;
            
            QColor previewColor = m_currentColor;
            previewColor.setAlpha(180);
            QPen arcPen(previewColor);
            arcPen.setWidthF(1.5 / currentScale);
            arcPen.setCosmetic(true);
            painter.setPen(arcPen);
            painter.setBrush(Qt::NoBrush);
            QRectF arcRect(p1Pt.x() - radius, p1Pt.y() - radius, radius * 2, radius * 2);
            painter.drawArc(arcRect, int(startAngle * 16), int(span * 16));
        }
    }
    else if (m_mode == ArcCreationMode::ThreePoints) {
        // Линии между точками
        painter.setPen(guidePen);
        painter.drawLine(p1Pt, currentPt);
        
        if (m_step >= 2) {
            QPointF p2Pt(m_p2.getX(), m_p2.getY());
            painter.drawLine(p1Pt, p2Pt);
            painter.drawLine(p2Pt, currentPt);
        }
    }
    
    // ЖИРНЫЕ МАРКЕРЫ ТОЧЕК
    QPen markerPen(Qt::white, 2.0 / currentScale);
    markerPen.setCosmetic(true);
    painter.setPen(markerPen);
    painter.setBrush(m_currentColor);
    const double markerSize = 6.0 / currentScale;
    
    painter.drawEllipse(p1Pt, markerSize, markerSize);
    
    if (m_step >= 2) {
        QPointF p2Pt(m_p2.getX(), m_p2.getY());
        painter.drawEllipse(p2Pt, markerSize, markerSize);
    }
}
