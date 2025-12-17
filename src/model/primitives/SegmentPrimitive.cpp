#include "SegmentPrimitive.h"
#include "LineStyleManager.h" //для отрисовки
#include <cmath>

SegmentPrimitive::SegmentPrimitive(const PointPrimitive& start, const PointPrimitive& end)
    : m_start(start), m_end(end) {}

const PointPrimitive& SegmentPrimitive::getStart() const { return m_start; }
void SegmentPrimitive::setStart(const PointPrimitive& point) { m_start = point; }

const PointPrimitive& SegmentPrimitive::getEnd() const { return m_end; }
void SegmentPrimitive::setEnd(const PointPrimitive& point) { m_end = point; }

QRectF SegmentPrimitive::getBoundingBox() const
{
    qreal minX = std::min(m_start.getX(), m_end.getX());
    qreal maxX = std::max(m_start.getX(), m_end.getX());
    qreal minY = std::min(m_start.getY(), m_end.getY());
    qreal maxY = std::max(m_start.getY(), m_end.getY());
    return QRectF(QPointF(minX, minY), QPointF(maxX, maxY));
}

void SegmentPrimitive::draw(QPainter& painter, bool isSelected) const
{
    //примитив использует глобальный менеджер стилей для отрисовки
    LineStyleManager::instance().drawLine(
        painter,
        QPointF(m_start.getX(), m_start.getY()),
        QPointF(m_end.getX(), m_end.getY()),
        getLineType(),
        getColor(),
        isSelected
        );
}

bool SegmentPrimitive::hitTest(const QPointF& point, double tolerance) const
{
    QPointF p1(m_start.getX(), m_start.getY());
    QPointF p2(m_end.getX(), m_end.getY());
    QLineF line(p1, p2);

    //если отрезок нулевой длины, проверяем как точку
    if (line.length() < 1e-9) return QLineF(point, p1).length() <= tolerance;

    //математика расстояния от точки до отрезка
    QPointF vecLine = p2 - p1;
    QPointF vecPoint = point - p1;
    double lineLen2 = QPointF::dotProduct(vecLine, vecLine);
    double t = QPointF::dotProduct(vecPoint, vecLine) / lineLen2;

    double dist;
    if (t < 0.0) dist = QLineF(point, p1).length();
    else if (t > 1.0) dist = QLineF(point, p2).length();
    else {
        QPointF projection = p1 + t * vecLine;
        dist = QLineF(point, projection).length();
    }

    return dist <= tolerance;
}

bool SegmentPrimitive::intersects(const QRectF& rect) const
{
    QLineF line(m_start.getX(), m_start.getY(), m_end.getX(), m_end.getY());

    //проверка: концы внутри или линия пересекает одну из сторон
    if (rect.contains(line.p1()) || rect.contains(line.p2())) return true;

    //простая проверка через BoundingBox (для повышения производительности)
    //для идеальной точности здесь можно добавить пересечение линии со сторонами rect
    return rect.intersects(getBoundingBox());
}

bool SegmentPrimitive::inside(const QRectF& rect) const
{
    QPointF p1(m_start.getX(), m_start.getY());
    QPointF p2(m_end.getX(), m_end.getY());
    return rect.contains(p1) && rect.contains(p2);
}

QVector<QPointF> SegmentPrimitive::getSnapPoints() const
{
    QVector<QPointF> points;
    QPointF p1(m_start.getX(), m_start.getY());
    QPointF p2(m_end.getX(), m_end.getY());

    points.append(p1); //начало
    points.append(p2); //конец
    points.append((p1 + p2) / 2.0); //середина
    return points;
}
