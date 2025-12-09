#include "EllipseCreationTool.h"
#include "ViewportPanelWidget.h"
#include <QPainter>
#include <QMouseEvent>
#include <QtMath>

EllipseCreationTool::EllipseCreationTool(QObject* parent) : BaseCreationTool(parent) {}

void EllipseCreationTool::onMousePress(QMouseEvent* event, Scene*, ViewportPanelWidget* viewport) {
    if (event->button() == Qt::LeftButton) {
        QPointF snapped = viewport->getSnappedPoint(event->position());
        PointPrimitive pt(snapped.x(), snapped.y());

        if (m_step == 0) {
            m_center = pt;
            m_currentPos = pt;
            m_step = 1;
        } else if (m_step == 1) {
            // Первая полуось
            m_axisPoint1 = pt;
            m_step = 2;
        } else if (m_step == 2) {
            // Вторая полуось (длина перпендикуляра от мыши до первой оси)
            // 1. Радиус X (расстояние Center -> Axis1)
            double rx = QLineF(m_center.getX(), m_center.getY(), m_axisPoint1.getX(), m_axisPoint1.getY()).length();
            double angle = QLineF(m_center.getX(), m_center.getY(), m_axisPoint1.getX(), m_axisPoint1.getY()).angle();

            // 2. Радиус Y (расстояние от мыши до линии первой оси)
            // Упрощенно: расстояние от центра до мыши (но это неправильно для эллипса)
            // Реализуем "Ортодоксальный" метод: Rx задается точкой, Ry задается длиной перпендикуляра.

            // Проецируем вектор (Center->Mouse) на перпендикуляр к оси
            // Повернем точку Mouse на -angle, тогда ось X станет нашей осью. Y мыши станет Ry.

            double dx = pt.getX() - m_center.getX();
            double dy = pt.getY() - m_center.getY();
            double radAngle = -angle * M_PI / 180.0; // В радианы, обратный поворот

            // double rotX = dx * cos(rad) - dy * sin(rad);
            double rotY = dx * std::sin(radAngle) + dy * std::cos(radAngle);
            double ry = std::abs(rotY);

            if (rx > 0 && ry > 0) {
                // angle возвращает 0..360 CCW (Qt coordinates inverted Y logic mess)
                // QPainter rotate CW. QLineF angle 0 is 3 o'clock.
                // В эллипсе rotation передаем как -angle (CCW -> CW for painter)
                emit ellipseDataReady(m_center, rx, ry, -angle);
            }
            reset();
        }
        viewport->update();
    } else if (event->button() == Qt::RightButton) {
        reset();
        viewport->update();
    }
}

void EllipseCreationTool::onMouseMove(QMouseEvent* event, Scene*, ViewportPanelWidget* viewport) {
    if (m_step > 0) {
        QPointF snapped = viewport->getSnappedPoint(event->position());
        m_currentPos = PointPrimitive(snapped.x(), snapped.y());
        viewport->update();
    }
}

void EllipseCreationTool::onMouseRelease(QMouseEvent*, Scene*, ViewportPanelWidget*) {}
void EllipseCreationTool::reset() { m_step = 0; }

void EllipseCreationTool::onPaint(QPainter& painter) {
    if (m_step == 0) return;

    QColor c = m_currentColor; c.setAlpha(150);
    painter.setPen(QPen(c, 1, Qt::DashLine));

    if (m_step == 1) {
        painter.drawLine(QPointF(m_center.getX(), m_center.getY()), QPointF(m_currentPos.getX(), m_currentPos.getY()));
    } else if (m_step == 2) {
        // Рисуем эллипс
        painter.save();
        painter.translate(m_center.getX(), m_center.getY());
        double angle = QLineF(m_center.getX(), m_center.getY(), m_axisPoint1.getX(), m_axisPoint1.getY()).angle();
        painter.rotate(-angle);

        double rx = QLineF(m_center.getX(), m_center.getY(), m_axisPoint1.getX(), m_axisPoint1.getY()).length();

        // Расчет ry (копия из onMousePress)
        double dx = m_currentPos.getX() - m_center.getX();
        double dy = m_currentPos.getY() - m_center.getY();
        double radAngle = -angle * M_PI / 180.0;
        double ry = std::abs(dx * std::sin(radAngle) + dy * std::cos(radAngle));

        painter.drawEllipse(QPointF(0,0), rx, ry);
        painter.restore();
    }
}
