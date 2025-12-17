#include "EllipsePrimitive.h"
#include "LineStyleManager.h"
#include <algorithm>
#include <cmath>
#include <QTransform>

EllipsePrimitive::EllipsePrimitive(const PointPrimitive& center, double rx, double ry, double rot)
    : m_center(center), m_radiusX(rx), m_radiusY(ry), m_rotation(rot) {}

QRectF EllipsePrimitive::getBoundingBox() const {
    // Упрощенный AABB (берем максимальный радиус)
    double maxR = std::max(m_radiusX, m_radiusY);
    return QRectF(m_center.getX() - maxR, m_center.getY() - maxR, maxR * 2, maxR * 2);
}

void EllipsePrimitive::draw(QPainter& painter, bool isSelected) const {
    painter.save();
    // Трансформация: перенос и поворот
    painter.translate(m_center.getX(), m_center.getY());
    painter.rotate(m_rotation);

    // Рисуем эллипс в локальных координатах (0,0)
    // Используем LineStyleManager для поддержки стилей
    LineStyleManager::instance().drawEllipse(
        painter,
        QPointF(0, 0),
        m_radiusX, m_radiusY,
        getLineType(),
        getColor(),
        isSelected
        );

    painter.restore();
}

bool EllipsePrimitive::hitTest(const QPointF& point, double tolerance) const {
    // Переводим точку в локальную систему координат эллипса
    QTransform t;
    t.rotate(-m_rotation);
    t.translate(-m_center.getX(), -m_center.getY());
    QPointF local = t.map(point);

    // Уравнение эллипса: (x/a)^2 + (y/b)^2 = 1
    if (m_radiusX == 0 || m_radiusY == 0) return false;

    double x = local.x();
    double y = local.y();
    double eq = (x*x)/(m_radiusX*m_radiusX) + (y*y)/(m_radiusY*m_radiusY);
    double dist = std::sqrt(eq); // Дистанция в "единицах эллипса"

    // Это приближенная проверка. Для точной проверки "расстояния до эллипса"
    // нужна сложная математика. Но для UI клика достаточно проверить,
    // что мы находимся "близко к контуру".
    // Преобразуем tolerance в примерный масштаб.

    // Если точка лежит на эллипсе, dist = 1.
    // Если dist ~ 1, то попали.

    // Упрощение: проверяем попадание в кольцо вокруг эллипса
    // Масштабируем tolerance относительно среднего радиуса
    double avgR = (m_radiusX + m_radiusY) / 2.0;
    double normTol = tolerance / avgR;

    return std::abs(dist - 1.0) <= normTol;
}

bool EllipsePrimitive::intersects(const QRectF& rect) const {
    return rect.intersects(getBoundingBox());
}

bool EllipsePrimitive::inside(const QRectF& rect) const {
    return rect.contains(getBoundingBox());
}

QVector<QPointF> EllipsePrimitive::getSnapPoints() const {
    QVector<QPointF> points;
    points.append(QPointF(m_center.getX(), m_center.getY())); // Центр

    // Точки квадрантов (с учетом поворота)
    QTransform t;
    t.translate(m_center.getX(), m_center.getY());
    t.rotate(m_rotation);

    points.append(t.map(QPointF(m_radiusX, 0)));
    points.append(t.map(QPointF(-m_radiusX, 0)));
    points.append(t.map(QPointF(0, m_radiusY)));
    points.append(t.map(QPointF(0, -m_radiusY)));

    return points;
}
