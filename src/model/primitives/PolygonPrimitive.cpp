#include "PolygonPrimitive.h"
#include "LineStyleManager.h"

#include <QPainter>
#include <cmath>

PolygonPrimitive::PolygonPrimitive(const PointPrimitive& center, double radius, int sides,
                                   PolygonType type, double rotation)
    : m_center(center), m_radius(radius), m_sides(sides >= 3 ? sides : 3),
      m_polygonType(type), m_rotation(rotation)
{
}

QVector<QPointF> PolygonPrimitive::getVertices() const
{
    QVector<QPointF> vertices;
    vertices.reserve(m_sides);
    
    double cx = m_center.getX();
    double cy = m_center.getY();
    double angleStep = 2.0 * M_PI / m_sides;
    double startAngle = m_rotation * M_PI / 180.0;
    
    double effectiveRadius = m_radius;
    
    // Для описанного многоугольника радиус = расстояние до середины стороны
    // Нужно пересчитать в радиус до вершины
    if (m_polygonType == PolygonType::Circumscribed) {
        effectiveRadius = m_radius / std::cos(M_PI / m_sides);
    }
    
    for (int i = 0; i < m_sides; ++i) {
        double angle = startAngle + i * angleStep;
        double x = cx + effectiveRadius * std::cos(angle);
        double y = cy + effectiveRadius * std::sin(angle);
        vertices.append(QPointF(x, y));
    }
    
    return vertices;
}

void PolygonPrimitive::draw(QPainter& painter, bool isSelected) const
{
    QVector<QPointF> vertices = getVertices();
    if (vertices.isEmpty()) return;
    
    // Устанавливаем стиль через LineStyleManager
    LineStyleManager& lsm = LineStyleManager::instance();
    QPen pen = lsm.getPen(getLineType(), getColor(), isSelected);
    painter.setPen(pen);
    painter.setBrush(Qt::NoBrush);
    
    // Рисуем полигон
    QPolygonF polygon(vertices);
    polygon.append(vertices.first()); // Замыкаем
    
    // Для специальных стилей линий рисуем по сторонам
    int lineType = getLineType();
    if (lineType >= static_cast<int>(LineType::SolidWave)) {
        // Рисуем каждую сторону отдельно
        for (int i = 0; i < vertices.size(); ++i) {
            QPointF p1 = vertices[i];
            QPointF p2 = vertices[(i + 1) % vertices.size()];
            lsm.drawLine(painter, p1, p2, lineType, getColor());
        }
    } else {
        // Обычная отрисовка
        painter.drawPolygon(polygon);
    }
}

QRectF PolygonPrimitive::getBoundingBox() const
{
    QVector<QPointF> vertices = getVertices();
    if (vertices.isEmpty()) {
        return QRectF(m_center.getX(), m_center.getY(), 0, 0);
    }
    
    double minX = vertices[0].x(), maxX = vertices[0].x();
    double minY = vertices[0].y(), maxY = vertices[0].y();
    
    for (const auto& v : vertices) {
        minX = std::min(minX, v.x());
        maxX = std::max(maxX, v.x());
        minY = std::min(minY, v.y());
        maxY = std::max(maxY, v.y());
    }
    
    return QRectF(QPointF(minX, minY), QPointF(maxX, maxY));
}

bool PolygonPrimitive::hitTest(const QPointF& point, double tolerance) const
{
    QVector<QPointF> vertices = getVertices();
    
    // Проверка на близость к каждой стороне
    for (int i = 0; i < vertices.size(); ++i) {
        QPointF p1 = vertices[i];
        QPointF p2 = vertices[(i + 1) % vertices.size()];
        
        // Расстояние от точки до отрезка
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

bool PolygonPrimitive::intersects(const QRectF& rect) const
{
    // Проверка пересечения bbox
    return getBoundingBox().intersects(rect);
}

bool PolygonPrimitive::inside(const QRectF& rect) const
{
    // Все вершины должны быть внутри
    QVector<QPointF> vertices = getVertices();
    for (const auto& v : vertices) {
        if (!rect.contains(v)) return false;
    }
    return true;
}

QVector<QPointF> PolygonPrimitive::getSnapPoints() const
{
    QVector<QPointF> snaps;
    QVector<QPointF> vertices = getVertices();
    
    // Все вершины (endpoints)
    snaps.append(vertices);
    
    // Центр
    snaps.append(QPointF(m_center.getX(), m_center.getY()));
    
    // Середины сторон
    for (int i = 0; i < vertices.size(); ++i) {
        QPointF p1 = vertices[i];
        QPointF p2 = vertices[(i + 1) % vertices.size()];
        snaps.append((p1 + p2) / 2.0);
    }
    
    return snaps;
}
