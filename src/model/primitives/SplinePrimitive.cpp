#include "SplinePrimitive.h"
#include "LineStyleManager.h"

#include <QPainter>
#include <QPainterPath>
#include <cmath>

SplinePrimitive::SplinePrimitive()
{
}

SplinePrimitive::SplinePrimitive(const QVector<QPointF>& controlPoints)
    : m_controlPoints(controlPoints)
{
}

void SplinePrimitive::addControlPoint(const QPointF& point)
{
    m_controlPoints.append(point);
}

void SplinePrimitive::insertControlPoint(int index, const QPointF& point)
{
    if (index >= 0 && index <= m_controlPoints.size()) {
        m_controlPoints.insert(index, point);
    }
}

void SplinePrimitive::removeControlPoint(int index)
{
    if (index >= 0 && index < m_controlPoints.size()) {
        m_controlPoints.removeAt(index);
    }
}

void SplinePrimitive::moveControlPoint(int index, const QPointF& newPos)
{
    if (index >= 0 && index < m_controlPoints.size()) {
        m_controlPoints[index] = newPos;
    }
}

QPointF SplinePrimitive::catmullRom(const QPointF& p0, const QPointF& p1,
                                     const QPointF& p2, const QPointF& p3, double t) const
{
    // Catmull-Rom spline interpolation
    double t2 = t * t;
    double t3 = t2 * t;
    
    double b0 = -0.5 * t3 + t2 - 0.5 * t;
    double b1 = 1.5 * t3 - 2.5 * t2 + 1.0;
    double b2 = -1.5 * t3 + 2.0 * t2 + 0.5 * t;
    double b3 = 0.5 * t3 - 0.5 * t2;
    
    return QPointF(
        b0 * p0.x() + b1 * p1.x() + b2 * p2.x() + b3 * p3.x(),
        b0 * p0.y() + b1 * p1.y() + b2 * p2.y() + b3 * p3.y()
    );
}

QVector<QPointF> SplinePrimitive::calculateSplinePoints(int segmentsPerCurve) const
{
    QVector<QPointF> result;
    
    int n = m_controlPoints.size();
    if (n < 2) return result;
    
    if (n == 2) {
        // Просто линия между двумя точками
        result.append(m_controlPoints[0]);
        result.append(m_controlPoints[1]);
        return result;
    }
    
    // Для каждого сегмента сплайна
    for (int i = 0; i < n - 1; ++i) {
        // Получаем 4 контрольные точки для Catmull-Rom
        QPointF p0 = (i == 0) ? 
            (m_closed ? m_controlPoints[n - 1] : m_controlPoints[0]) : 
            m_controlPoints[i - 1];
        QPointF p1 = m_controlPoints[i];
        QPointF p2 = m_controlPoints[i + 1];
        QPointF p3 = (i == n - 2) ? 
            (m_closed ? m_controlPoints[0] : m_controlPoints[n - 1]) : 
            m_controlPoints[i + 2];
        
        // Генерируем точки на сегменте
        for (int j = 0; j <= segmentsPerCurve; ++j) {
            double t = static_cast<double>(j) / segmentsPerCurve;
            result.append(catmullRom(p0, p1, p2, p3, t));
        }
    }
    
    // Замыкание
    if (m_closed && n >= 3) {
        QPointF p0 = m_controlPoints[n - 2];
        QPointF p1 = m_controlPoints[n - 1];
        QPointF p2 = m_controlPoints[0];
        QPointF p3 = m_controlPoints[1];
        
        for (int j = 0; j <= segmentsPerCurve; ++j) {
            double t = static_cast<double>(j) / segmentsPerCurve;
            result.append(catmullRom(p0, p1, p2, p3, t));
        }
    }
    
    return result;
}

void SplinePrimitive::draw(QPainter& painter, bool isSelected) const
{
    if (m_controlPoints.size() < 2) return;
    
    LineStyleManager& lsm = LineStyleManager::instance();
    
    // Рисуем сплайн сегмент за сегментом для поддержки всех стилей линий
    QVector<QPointF> splinePoints = calculateSplinePoints();
    
    if (splinePoints.size() >= 2) {
        // Для каждого сегмента используем LineStyleManager
        for (int i = 0; i < splinePoints.size() - 1; ++i) {
            lsm.drawLine(painter, splinePoints[i], splinePoints[i + 1],
                         getLineType(), getColor(), isSelected);
        }
    }
    
    // Рисуем контрольные точки при выделении
    if (isSelected) {
        QPen ctrlPen(Qt::yellow);
        ctrlPen.setWidth(1);
        painter.setPen(ctrlPen);
        painter.setBrush(Qt::yellow);
        
        for (const auto& pt : m_controlPoints) {
            painter.drawEllipse(pt, 4.0, 4.0);
        }
        
        // Соединяем контрольные точки пунктиром
        QPen guidePen(Qt::gray);
        guidePen.setStyle(Qt::DotLine);
        painter.setPen(guidePen);
        painter.setBrush(Qt::NoBrush);
        
        for (int i = 0; i < m_controlPoints.size() - 1; ++i) {
            painter.drawLine(m_controlPoints[i], m_controlPoints[i + 1]);
        }
        if (m_closed && m_controlPoints.size() >= 2) {
            painter.drawLine(m_controlPoints.last(), m_controlPoints.first());
        }
    }
}

QRectF SplinePrimitive::getBoundingBox() const
{
    if (m_controlPoints.isEmpty()) return QRectF();
    
    QVector<QPointF> splinePoints = calculateSplinePoints();
    if (splinePoints.isEmpty()) return QRectF();
    
    double minX = splinePoints[0].x(), maxX = splinePoints[0].x();
    double minY = splinePoints[0].y(), maxY = splinePoints[0].y();
    
    for (const auto& pt : splinePoints) {
        minX = std::min(minX, pt.x());
        maxX = std::max(maxX, pt.x());
        minY = std::min(minY, pt.y());
        maxY = std::max(maxY, pt.y());
    }
    
    return QRectF(QPointF(minX, minY), QPointF(maxX, maxY));
}

bool SplinePrimitive::hitTest(const QPointF& point, double tolerance) const
{
    QVector<QPointF> splinePoints = calculateSplinePoints();
    
    // Проверяем близость к каждому сегменту сплайна
    for (int i = 0; i < splinePoints.size() - 1; ++i) {
        QPointF p1 = splinePoints[i];
        QPointF p2 = splinePoints[i + 1];
        
        QPointF vec = p2 - p1;
        QPointF vecPoint = point - p1;
        double len2 = QPointF::dotProduct(vec, vec);
        
        if (len2 == 0.0) {
            if (QLineF(point, p1).length() < tolerance) return true;
            continue;
        }
        
        double t = QPointF::dotProduct(vecPoint, vec) / len2;
        t = std::max(0.0, std::min(1.0, t));
        QPointF projection = p1 + t * vec;
        
        if (QLineF(point, projection).length() < tolerance) {
            return true;
        }
    }
    
    return false;
}

bool SplinePrimitive::intersects(const QRectF& rect) const
{
    return getBoundingBox().intersects(rect);
}

bool SplinePrimitive::inside(const QRectF& rect) const
{
    for (const auto& pt : m_controlPoints) {
        if (!rect.contains(pt)) return false;
    }
    return true;
}

QVector<QPointF> SplinePrimitive::getSnapPoints() const
{
    QVector<QPointF> snaps;
    
    // Все контрольные точки
    snaps.append(m_controlPoints);
    
    // Можно добавить середины сегментов если нужно
    
    return snaps;
}
