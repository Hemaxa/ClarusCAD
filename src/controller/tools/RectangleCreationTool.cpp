#include "RectangleCreationTool.h"
#include "ViewportPanelWidget.h"
#include <QPainter>
#include <QtMath>
#include <QMouseEvent>

RectangleCreationTool::RectangleCreationTool(QObject* parent) : BaseCreationTool(parent) {}

void RectangleCreationTool::setCreationMode(RectangleCreationMode mode) {
    m_mode = mode;
    reset();
}

void RectangleCreationTool::onMousePress(QMouseEvent* event, Scene* scene, ViewportPanelWidget* viewport) {
    if (event->button() == Qt::LeftButton) {
        QPointF snapped = viewport->getSnappedPoint(event->position());
        PointPrimitive pt(snapped.x(), snapped.y());

        if (m_mode == RectangleCreationMode::TwoPoints) {
            // По диагонали: P1 -> P2
            if (m_step == 0) {
                m_p1 = pt;
                m_currentPos = pt;
                m_step = 1;
            } else {
                double w = std::abs(pt.getX() - m_p1.getX());
                double h = std::abs(pt.getY() - m_p1.getY());
                double cx = (pt.getX() + m_p1.getX()) / 2.0;
                double cy = (pt.getY() + m_p1.getY()) / 2.0;

                if (w > 0 && h > 0) {
                    emit rectangleDataReady(PointPrimitive(cx, cy), w, h, 0.0);
                }
                reset();
            }
        }
        else if (m_mode == RectangleCreationMode::CenterSize) {
            // Центр -> Угол
            if (m_step == 0) {
                m_p1 = pt; // Это центр
                m_currentPos = pt;
                m_step = 1;
            } else {
                double w = std::abs(pt.getX() - m_p1.getX()) * 2.0;
                double h = std::abs(pt.getY() - m_p1.getY()) * 2.0;

                if (w > 0 && h > 0) {
                    emit rectangleDataReady(m_p1, w, h, 0.0);
                }
                reset();
            }
        }
        else if (m_mode == RectangleCreationMode::PointSize) {
            // Точка (угол) -> Противоположный угол (ширина и высота откладываются в одну сторону)
            // Отличие от TwoPoints: точка - это угол, а не центр диагонали
            if (m_step == 0) {
                m_p1 = pt; // Это угол прямоугольника
                m_currentPos = pt;
                m_step = 1;
            } else {
                double w = pt.getX() - m_p1.getX();
                double h = pt.getY() - m_p1.getY();
                
                // Центр будет смещен от угла на половину ширины и высоты
                double cx = m_p1.getX() + w / 2.0;
                double cy = m_p1.getY() + h / 2.0;

                if (std::abs(w) > 0 && std::abs(h) > 0) {
                    emit rectangleDataReady(PointPrimitive(cx, cy), std::abs(w), std::abs(h), 0.0);
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

void RectangleCreationTool::onMouseMove(QMouseEvent* event, Scene* scene, ViewportPanelWidget* viewport) {
    if (m_step > 0) {
        QPointF snapped = viewport->getSnappedPoint(event->position());
        m_currentPos = PointPrimitive(snapped.x(), snapped.y());
        viewport->update();
    }
}

void RectangleCreationTool::onMouseRelease(QMouseEvent*, Scene*, ViewportPanelWidget*) {}
void RectangleCreationTool::reset() { m_step = 0; }

void RectangleCreationTool::onPaint(QPainter& painter) {
    if (m_step == 0) return;

    // Стандартный стиль для служебных линий
    QPen guidePen(Qt::white);
    guidePen.setStyle(Qt::DashLine);
    guidePen.setWidthF(1.0);
    
    QPointF p1Pt(m_p1.getX(), m_p1.getY());
    QPointF currentPt(m_currentPos.getX(), m_currentPos.getY());
    
    // Предпросмотр прямоугольника в цвете
    QColor previewColor = m_currentColor;
    previewColor.setAlpha(180);
    QPen rectPen(previewColor);
    rectPen.setWidthF(1.5);

    if (m_mode == RectangleCreationMode::TwoPoints) {
        double w = currentPt.x() - p1Pt.x();
        double h = currentPt.y() - p1Pt.y();
        
        // Линия диагонали (белая пунктирная)
        painter.setPen(guidePen);
        painter.drawLine(p1Pt, currentPt);
        
        // Предпросмотр прямоугольника
        painter.setPen(rectPen);
        painter.setBrush(Qt::NoBrush);
        painter.drawRect(QRectF(p1Pt.x(), p1Pt.y(), w, h));
    }
    else if (m_mode == RectangleCreationMode::CenterSize) {
        double halfW = std::abs(currentPt.x() - p1Pt.x());
        double halfH = std::abs(currentPt.y() - p1Pt.y());
        
        // Линия от центра к углу (белая пунктирная)
        painter.setPen(guidePen);
        painter.drawLine(p1Pt, currentPt);
        
        // Предпросмотр прямоугольника
        painter.setPen(rectPen);
        painter.setBrush(Qt::NoBrush);
        painter.drawRect(QRectF(p1Pt.x() - halfW, p1Pt.y() - halfH, halfW * 2, halfH * 2));
    }
    else if (m_mode == RectangleCreationMode::PointSize) {
        double w = currentPt.x() - p1Pt.x();
        double h = currentPt.y() - p1Pt.y();
        
        // Линия диагонали (белая пунктирная)
        painter.setPen(guidePen);
        painter.drawLine(p1Pt, currentPt);
        
        // Предпросмотр прямоугольника (от угла P1)
        painter.setPen(rectPen);
        painter.setBrush(Qt::NoBrush);
        painter.drawRect(QRectF(p1Pt.x(), p1Pt.y(), w, h));
    }
    
    // ЖИРНЫЙ МАРКЕР ПЕРВОЙ ТОЧКИ
    painter.setPen(QPen(Qt::white, 2.0));
    painter.setBrush(m_currentColor);
    painter.drawEllipse(p1Pt, 6, 6);
}
