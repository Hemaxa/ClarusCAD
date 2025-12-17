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
        else if (m_mode == RectangleCreationMode::ThreePoints) {
            // P1 (угол) -> P2 (ширина/угол) -> P3 (высота)
            if (m_step == 0) {
                m_p1 = pt;
                m_currentPos = pt;
                m_step = 1;
            } else if (m_step == 1) {
                m_p2 = pt;
                if (QLineF(m_p1.getX(), m_p1.getY(), m_p2.getX(), m_p2.getY()).length() > 0) {
                    m_step = 2;
                }
            } else {
                // Вычисляем ширину (P1-P2)
                QLineF vecW(m_p1.getX(), m_p1.getY(), m_p2.getX(), m_p2.getY());
                double width = vecW.length();
                double rotation = vecW.angle(); // 0..360
                // В Qt angle() возвращает угол 0..360 против часовой стрелки от 3 часов (обычно),
                // но в QPainter.rotate() нам нужны градусы. QLineF::angle() подходит. Note: Qt Y inverted.

                // Высота - проекция вектора (P2-Mouse) на перпендикуляр к ширине?
                // Упрощенно: расстояние от мыши до линии P1-P2

                // Вектор P2 -> Mouse
                QPointF p2(m_p2.getX(), m_p2.getY());
                QPointF p3(pt.getX(), pt.getY());
                QLineF vecH_raw(p2, p3);

                // Проекция на нормаль
                // Для простоты CAD построения "3 точки" обычно P1-P2 это одна сторона,
                // а P3 просто задает вытягивание перпендикулярно.

                double height = QLineF(p3, vecW.pointAt(
                                               (QPointF::dotProduct(p3 - vecW.p1(), vecW.p2() - vecW.p1())) / (width * width)
                                               ) * width + vecW.p1()).length(); // Расстояние от точки до прямой

                // Но лучше использовать distanceToLine для точности? Нет в QLineF до Qt 5.14+
                // Реализуем высоту как расстояние от P3 до прямой (P1, P2)

                // Упростим: Высота = длина перпендикуляра.
                // Центр прямоугольника смещается от середины P1-P2 на половину высоты вдоль нормали.

                // Вектор ширины
                double dx = m_p2.getX() - m_p1.getX();
                double dy = m_p2.getY() - m_p1.getY();

                // Нормаль (-dy, dx) или (dy, -dx)
                // Определяем знак высоты через векторное произведение
                double crossProduct = dx * (pt.getY() - m_p1.getY()) - dy * (pt.getX() - m_p1.getX());
                double h_signed = crossProduct / width;
                double h_final = std::abs(h_signed);

                // Центр
                double midX = (m_p1.getX() + m_p2.getX()) / 2.0;
                double midY = (m_p1.getY() + m_p2.getY()) / 2.0;

                // Смещаем центр по нормали
                // Нормализованный вектор перпендикуляра (-dy, dx) / width
                double nx = -dy / width;
                double ny = dx / width;

                double cx = midX + nx * (h_signed / 2.0);
                double cy = midY + ny * (h_signed / 2.0);

                emit rectangleDataReady(PointPrimitive(cx, cy), width, h_final, -rotation); // Qt rotation is CW? Check logic.
                // QPainter::rotate поворачивает по часовой
                // QLineF::angle возвращает 0..360 CCW
                // Значит -rotation

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
    else if (m_mode == RectangleCreationMode::ThreePoints) {
        // Линии между точками (белые пунктирные)
        painter.setPen(guidePen);
        
        if (m_step == 1) {
            painter.drawLine(p1Pt, currentPt);
        } else if (m_step == 2) {
            QPointF p2Pt(m_p2.getX(), m_p2.getY());
            painter.drawLine(p1Pt, p2Pt);
            painter.drawLine(p2Pt, currentPt);
            
            // Маркер для P2
            painter.setPen(QPen(Qt::white, 2.0));
            painter.setBrush(m_currentColor);
            painter.drawEllipse(p2Pt, 6, 6);
        }
    }
    
    // ЖИРНЫЙ МАРКЕР ПЕРВОЙ ТОЧКИ
    painter.setPen(QPen(Qt::white, 2.0));
    painter.setBrush(m_currentColor);
    painter.drawEllipse(p1Pt, 6, 6);
}
