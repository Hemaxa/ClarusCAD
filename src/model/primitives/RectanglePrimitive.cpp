#include "RectanglePrimitive.h"
#include "LineStyleManager.h"
#include <QPainterPath>
#include <QTransform>
#include <QtMath>

RectanglePrimitive::RectanglePrimitive(const PointPrimitive& center, double width, double height, double rotation)
    : m_center(center), m_width(width), m_height(height), m_rotation(rotation) {}

QRectF RectanglePrimitive::getBoundingBox() const {
    // Вычисляем AABB повернутого прямоугольника
    return getTransformedPolygon().boundingRect();
}

QPolygonF RectanglePrimitive::getTransformedPolygon() const {
    QRectF baseRect(-m_width/2, -m_height/2, m_width, m_height);

    QTransform t;
    t.translate(m_center.getX(), m_center.getY());
    t.rotate(m_rotation);

    return t.map(QPolygonF(baseRect));
}

void RectanglePrimitive::draw(QPainter& painter, bool isSelected) const {
    // Формируем путь для отрисовки (учитывая скругления/фаски)
    QPainterPath path;
    QRectF rBase(-m_width/2, -m_height/2, m_width, m_height);

    if (m_cornerType == CornerType::Fillet && m_cornerRadius > 0) {
        path.addRoundedRect(rBase, m_cornerRadius, m_cornerRadius);
    }
    else if (m_cornerType == CornerType::Chamfer && m_cornerRadius > 0) {
        // Фаска вручную
        double w = m_width, h = m_height, r = m_cornerRadius;
        QPolygonF poly;
        poly << QPointF(-w/2 + r, -h/2) << QPointF(w/2 - r, -h/2)
             << QPointF(w/2, -h/2 + r) << QPointF(w/2, h/2 - r)
             << QPointF(w/2 - r, h/2) << QPointF(-w/2 + r, h/2)
             << QPointF(-w/2, h/2 - r) << QPointF(-w/2, -h/2 + r);
        poly.append(poly.first()); // Замыкаем
        path.addPolygon(poly);
    }
    else {
        path.addRect(rBase);
    }

    // Применяем трансформацию позиции и поворота
    QTransform t;
    t.translate(m_center.getX(), m_center.getY());
    t.rotate(m_rotation);
    path = t.map(path);

    // Рисуем используя LineStyleManager для поддержки всех типов линий (волны, зигзаги и т.д.)
    LineStyleManager::instance().drawPath(painter, path, getLineType(), getColor(), isSelected);
}

bool RectanglePrimitive::hitTest(const QPointF& point, double tolerance) const {
    // Трансформируем точку клика в локальную систему координат прямоугольника
    // (Обратное преобразование: сдвиг к (0,0) и поворот назад)
    QTransform t;
    t.rotate(-m_rotation);
    t.translate(-m_center.getX(), -m_center.getY());

    QPointF localPoint = t.map(point);

    // Теперь проверяем попадание в границы axis-aligned прямоугольника (-w/2, -h/2, w, h)
    double halfW = m_width / 2.0;
    double halfH = m_height / 2.0;

    // Проверка попадания в "рамку" (контур)
    // Расстояние до левой/правой стороны
    bool inY = (localPoint.y() >= -halfH - tolerance && localPoint.y() <= halfH + tolerance);
    bool hitVertical = inY && (std::abs(std::abs(localPoint.x()) - halfW) <= tolerance);

    // Расстояние до верхней/нижней стороны
    bool inX = (localPoint.x() >= -halfW - tolerance && localPoint.x() <= halfW + tolerance);
    bool hitHorizontal = inX && (std::abs(std::abs(localPoint.y()) - halfH) <= tolerance);

    return hitVertical || hitHorizontal;
}

bool RectanglePrimitive::intersects(const QRectF& rect) const {
    return rect.intersects(getBoundingBox());
}

bool RectanglePrimitive::inside(const QRectF& rect) const {
    return rect.contains(getBoundingBox());
}

QVector<QPointF> RectanglePrimitive::getSnapPoints() const {
    QVector<QPointF> points;
    points.append(QPointF(m_center.getX(), m_center.getY())); // Центр

    // Получаем углы
    QPolygonF poly = getTransformedPolygon();
    // poly содержит 5 точек (последняя повторяет первую для замкнутости), берем первые 4
    for(int i=0; i<4; ++i) {
        points.append(poly.at(i));
    }

    // Середины сторон
    for(int i=0; i<4; ++i) {
        QPointF p1 = poly.at(i);
        QPointF p2 = poly.at((i+1)%4);
        points.append((p1 + p2) / 2.0);
    }

    return points;
}
