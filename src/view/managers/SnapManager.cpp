#include "SnapManager.h"
#include "Scene.h"
#include "BasePrimitive.h"
#include "SegmentPrimitive.h"
#include "CirclePrimitive.h"
#include "ArcPrimitive.h"
#include "RectanglePrimitive.h"
#include "EllipsePrimitive.h"
#include "PolygonPrimitive.h"
#include "SplinePrimitive.h"

#include <cmath>
#include <algorithm>

SnapManager& SnapManager::instance()
{
    static SnapManager instance;
    return instance;
}

SnapManager::SnapManager() : QObject(nullptr)
{
    // По умолчанию включены все основные привязки
    m_enabledSnapTypes = static_cast<int>(SnapType::Endpoint) |
                         static_cast<int>(SnapType::Midpoint) |
                         static_cast<int>(SnapType::Center) |
                         static_cast<int>(SnapType::Quadrant) |
                         static_cast<int>(SnapType::Grid) |
                         static_cast<int>(SnapType::Intersection) |
                         static_cast<int>(SnapType::Perpendicular) |
                         static_cast<int>(SnapType::Tangent);
}

SnapPoint SnapManager::findNearestSnapPoint(const QPointF& mousePos, Scene* scene,
                                             double zoomFactor, double tolerance)
{
    QVector<SnapPoint> allPoints = findSnapPointsInRadius(mousePos, scene, zoomFactor, tolerance);
    
    if (allPoints.isEmpty()) {
        // Если нет привязок, возвращаем саму позицию
        SnapPoint noSnap;
        noSnap.position = mousePos;
        noSnap.type = SnapType::None;
        return noSnap;
    }
    
    // Сортировка по расстоянию
    std::sort(allPoints.begin(), allPoints.end());
    return allPoints.first();
}

QVector<SnapPoint> SnapManager::findSnapPointsInRadius(const QPointF& mousePos, Scene* scene,
                                                        double zoomFactor, double radius)
{
    QVector<SnapPoint> result;
    
    if (!scene) return result;
    
    // Пересчет радиуса в мировые координаты
    double worldRadius = radius / zoomFactor;
    
    // Сбор точек привязок от всех примитивов
    for (const auto& primitive : scene->getPrimitives()) {
        collectSnapPoints(mousePos, primitive.get(), worldRadius, result);
    }
    
    // Сбор точек пересечений (если включено)
    if (isSnapTypeEnabled(SnapType::Intersection)) {
        collectIntersections(mousePos, scene, worldRadius, result);
    }
    
    // Сбор перпендикуляров и касательных (если есть базовая точка)
    if (m_hasBasePoint) {
        if (isSnapTypeEnabled(SnapType::Perpendicular)) {
            collectPerpendiculars(mousePos, m_basePoint, scene, worldRadius, result);
        }
        if (isSnapTypeEnabled(SnapType::Tangent)) {
            collectTangents(mousePos, m_basePoint, scene, worldRadius, result);
        }
    }
    
    // Привязка к сетке (если включена)
    if (isSnapTypeEnabled(SnapType::Grid)) {
        SnapPoint gridSnap = snapToGrid(mousePos);
        double dist = QLineF(mousePos, gridSnap.position).length();
        if (dist < worldRadius) {
            gridSnap.distance = dist;
            result.append(gridSnap);
        }
    }
    
    return result;
}

void SnapManager::setSnapTypesEnabled(int snapFlags)
{
    m_enabledSnapTypes = snapFlags;
    emit snapTypesChanged();
}

int SnapManager::getSnapTypesEnabled() const
{
    return m_enabledSnapTypes;
}

void SnapManager::setSnapTypeEnabled(SnapType type, bool enabled)
{
    if (enabled) {
        m_enabledSnapTypes |= static_cast<int>(type);
    } else {
        m_enabledSnapTypes &= ~static_cast<int>(type);
    }
    emit snapTypesChanged();
}

bool SnapManager::isSnapTypeEnabled(SnapType type) const
{
    return (m_enabledSnapTypes & static_cast<int>(type)) != 0;
}

void SnapManager::setGridStep(double step)
{
    m_gridStep = step;
}

double SnapManager::getGridStep() const
{
    return m_gridStep;
}

void SnapManager::collectSnapPoints(const QPointF& mousePos, BasePrimitive* primitive,
                                     double tolerance, QVector<SnapPoint>& outPoints)
{
    if (!primitive) return;
    
    // Сбор различных типов точек привязок
    if (isSnapTypeEnabled(SnapType::Endpoint)) {
        collectEndpoints(primitive, outPoints);
    }
    if (isSnapTypeEnabled(SnapType::Midpoint)) {
        collectMidpoints(primitive, outPoints);
    }
    if (isSnapTypeEnabled(SnapType::Center)) {
        collectCenters(primitive, outPoints);
    }
    if (isSnapTypeEnabled(SnapType::Quadrant)) {
        collectQuadrants(primitive, outPoints);
    }
    
    // Расчет расстояний и фильтрация по толерансу
    for (int i = outPoints.size() - 1; i >= 0; --i) {
        outPoints[i].distance = QLineF(mousePos, outPoints[i].position).length();
        if (outPoints[i].distance > tolerance) {
            outPoints.removeAt(i);
        }
    }
}

void SnapManager::collectEndpoints(BasePrimitive* prim, QVector<SnapPoint>& out)
{
    SnapPoint sp;
    sp.type = SnapType::Endpoint;
    sp.source = prim;
    
    switch (prim->getType()) {
    case PrimitiveType::Segment: {
        auto* seg = static_cast<SegmentPrimitive*>(prim);
        sp.position = QPointF(seg->getStart().getX(), seg->getStart().getY());
        out.append(sp);
        sp.position = QPointF(seg->getEnd().getX(), seg->getEnd().getY());
        out.append(sp);
        break;
    }
    case PrimitiveType::Arc: {
        auto* arc = static_cast<ArcPrimitive*>(prim);
        double cx = arc->getCenter().getX();
        double cy = arc->getCenter().getY();
        double r = arc->getRadius();
        double startAngle = arc->getStartAngle() * M_PI / 180.0;
        double endAngle = (arc->getStartAngle() + arc->getSpanAngle()) * M_PI / 180.0;
        
        // Начало дуги
        sp.position = QPointF(cx + r * std::cos(startAngle), cy + r * std::sin(startAngle));
        out.append(sp);
        // Конец дуги
        sp.position = QPointF(cx + r * std::cos(endAngle), cy + r * std::sin(endAngle));
        out.append(sp);
        break;
    }
    case PrimitiveType::Rectangle: {
        auto* rect = static_cast<RectanglePrimitive*>(prim);
        // 4 угла прямоугольника
        QRectF r = rect->getBoundingBox();
        sp.position = r.topLeft();
        out.append(sp);
        sp.position = r.topRight();
        out.append(sp);
        sp.position = r.bottomLeft();
        out.append(sp);
        sp.position = r.bottomRight();
        out.append(sp);
        break;
    }
    case PrimitiveType::Polygon: {
        auto* poly = static_cast<PolygonPrimitive*>(prim);
        QVector<QPointF> vertices = poly->getVertices();
        for (const auto& v : vertices) {
            sp.position = v;
            out.append(sp);
        }
        break;
    }
    case PrimitiveType::Spline: {
        auto* spline = static_cast<SplinePrimitive*>(prim);
        QVector<QPointF> cpts = spline->getControlPoints();
        for (const auto& pt : cpts) {
            sp.position = pt;
            out.append(sp);
        }
        break;
    }
    default:
        break;
    }
}

void SnapManager::collectMidpoints(BasePrimitive* prim, QVector<SnapPoint>& out)
{
    SnapPoint sp;
    sp.type = SnapType::Midpoint;
    sp.source = prim;
    
    switch (prim->getType()) {
    case PrimitiveType::Segment: {
        auto* seg = static_cast<SegmentPrimitive*>(prim);
        double mx = (seg->getStart().getX() + seg->getEnd().getX()) / 2.0;
        double my = (seg->getStart().getY() + seg->getEnd().getY()) / 2.0;
        sp.position = QPointF(mx, my);
        out.append(sp);
        break;
    }
    case PrimitiveType::Arc: {
        auto* arc = static_cast<ArcPrimitive*>(prim);
        double cx = arc->getCenter().getX();
        double cy = arc->getCenter().getY();
        double r = arc->getRadius();
        // Середина дуги = начальный угол + половина span
        double midAngle = (arc->getStartAngle() + arc->getSpanAngle() / 2.0) * M_PI / 180.0;
        sp.position = QPointF(cx + r * std::cos(midAngle), cy + r * std::sin(midAngle));
        out.append(sp);
        break;
    }
    case PrimitiveType::Rectangle: {
        auto* rect = static_cast<RectanglePrimitive*>(prim);
        QRectF r = rect->getBoundingBox();
        // Середины 4 сторон
        sp.position = QPointF(r.center().x(), r.top()); // top
        out.append(sp);
        sp.position = QPointF(r.center().x(), r.bottom()); // bottom
        out.append(sp);
        sp.position = QPointF(r.left(), r.center().y()); // left
        out.append(sp);
        sp.position = QPointF(r.right(), r.center().y()); // right
        out.append(sp);
        break;
    }
    case PrimitiveType::Polygon: {
        auto* poly = static_cast<PolygonPrimitive*>(prim);
        QVector<QPointF> vertices = poly->getVertices();
        for (int i = 0; i < vertices.size(); ++i) {
            QPointF p1 = vertices[i];
            QPointF p2 = vertices[(i + 1) % vertices.size()];
            sp.position = (p1 + p2) / 2.0;
            out.append(sp);
        }
        break;
    }
    default:
        break;
    }
}

void SnapManager::collectCenters(BasePrimitive* prim, QVector<SnapPoint>& out)
{
    SnapPoint sp;
    sp.type = SnapType::Center;
    sp.source = prim;
    
    switch (prim->getType()) {
    case PrimitiveType::Circle: {
        auto* circle = static_cast<CirclePrimitive*>(prim);
        sp.position = QPointF(circle->getCenter().getX(), circle->getCenter().getY());
        out.append(sp);
        break;
    }
    case PrimitiveType::Arc: {
        auto* arc = static_cast<ArcPrimitive*>(prim);
        sp.position = QPointF(arc->getCenter().getX(), arc->getCenter().getY());
        out.append(sp);
        break;
    }
    case PrimitiveType::Ellipse: {
        auto* ellipse = static_cast<EllipsePrimitive*>(prim);
        sp.position = QPointF(ellipse->getCenter().getX(), ellipse->getCenter().getY());
        out.append(sp);
        break;
    }
    case PrimitiveType::Rectangle: {
        auto* rect = static_cast<RectanglePrimitive*>(prim);
        sp.position = rect->getBoundingBox().center();
        out.append(sp);
        break;
    }
    case PrimitiveType::Polygon: {
        auto* poly = static_cast<PolygonPrimitive*>(prim);
        sp.position = QPointF(poly->getCenter().getX(), poly->getCenter().getY());
        out.append(sp);
        break;
    }
    default:
        break;
    }
}

void SnapManager::collectQuadrants(BasePrimitive* prim, QVector<SnapPoint>& out)
{
    SnapPoint sp;
    sp.type = SnapType::Quadrant;
    sp.source = prim;
    
    auto addQuadrants = [&](double cx, double cy, double r) {
        sp.position = QPointF(cx + r, cy); // 0°
        out.append(sp);
        sp.position = QPointF(cx, cy + r); // 90°
        out.append(sp);
        sp.position = QPointF(cx - r, cy); // 180°
        out.append(sp);
        sp.position = QPointF(cx, cy - r); // 270°
        out.append(sp);
    };
    
    switch (prim->getType()) {
    case PrimitiveType::Circle: {
        auto* circle = static_cast<CirclePrimitive*>(prim);
        addQuadrants(circle->getCenter().getX(), circle->getCenter().getY(), circle->getRadius());
        break;
    }
    case PrimitiveType::Ellipse: {
        auto* ellipse = static_cast<EllipsePrimitive*>(prim);
        double cx = ellipse->getCenter().getX();
        double cy = ellipse->getCenter().getY();
        // Для эллипса квадранты = концы осей
        sp.position = QPointF(cx + ellipse->getRadiusX(), cy);
        out.append(sp);
        sp.position = QPointF(cx - ellipse->getRadiusX(), cy);
        out.append(sp);
        sp.position = QPointF(cx, cy + ellipse->getRadiusY());
        out.append(sp);
        sp.position = QPointF(cx, cy - ellipse->getRadiusY());
        out.append(sp);
        break;
    }
    default:
        break;
    }
}

SnapPoint SnapManager::snapToGrid(const QPointF& pos)
{
    SnapPoint sp;
    sp.type = SnapType::Grid;
    sp.source = nullptr;
    
    if (m_gridStep <= 0) {
        sp.position = pos;
        return sp;
    }
    
    double snappedX = std::round(pos.x() / m_gridStep) * m_gridStep;
    double snappedY = std::round(pos.y() / m_gridStep) * m_gridStep;
    sp.position = QPointF(snappedX, snappedY);
    
    return sp;
}

// === ГЕОМЕТРИЧЕСКИЕ ФУНКЦИИ ===

QVector<QPointF> SnapManager::findSegmentSegmentIntersection(const QPointF& a1, const QPointF& a2,
                                                              const QPointF& b1, const QPointF& b2)
{
    QVector<QPointF> result;
    
    double d1x = a2.x() - a1.x();
    double d1y = a2.y() - a1.y();
    double d2x = b2.x() - b1.x();
    double d2y = b2.y() - b1.y();
    
    double cross = d1x * d2y - d1y * d2x;
    if (std::abs(cross) < 1e-10) return result; // Параллельны
    
    double t = ((b1.x() - a1.x()) * d2y - (b1.y() - a1.y()) * d2x) / cross;
    double u = ((b1.x() - a1.x()) * d1y - (b1.y() - a1.y()) * d1x) / cross;
    
    // Проверяем что точка лежит на обоих отрезках
    if (t >= 0 && t <= 1 && u >= 0 && u <= 1) {
        result.append(QPointF(a1.x() + t * d1x, a1.y() + t * d1y));
    }
    
    return result;
}

QVector<QPointF> SnapManager::findSegmentCircleIntersection(const QPointF& p1, const QPointF& p2,
                                                             const QPointF& center, double radius)
{
    QVector<QPointF> result;
    
    double dx = p2.x() - p1.x();
    double dy = p2.y() - p1.y();
    double fx = p1.x() - center.x();
    double fy = p1.y() - center.y();
    
    double a = dx * dx + dy * dy;
    double b = 2.0 * (fx * dx + fy * dy);
    double c = fx * fx + fy * fy - radius * radius;
    
    double discriminant = b * b - 4.0 * a * c;
    if (discriminant < 0) return result;
    
    discriminant = std::sqrt(discriminant);
    double t1 = (-b - discriminant) / (2.0 * a);
    double t2 = (-b + discriminant) / (2.0 * a);
    
    if (t1 >= 0 && t1 <= 1) {
        result.append(QPointF(p1.x() + t1 * dx, p1.y() + t1 * dy));
    }
    if (t2 >= 0 && t2 <= 1 && std::abs(t1 - t2) > 1e-10) {
        result.append(QPointF(p1.x() + t2 * dx, p1.y() + t2 * dy));
    }
    
    return result;
}

QVector<QPointF> SnapManager::findCircleCircleIntersection(const QPointF& c1, double r1,
                                                            const QPointF& c2, double r2)
{
    QVector<QPointF> result;
    
    double dx = c2.x() - c1.x();
    double dy = c2.y() - c1.y();
    double d = std::sqrt(dx * dx + dy * dy);
    
    // Проверка на пересечение
    if (d > r1 + r2 || d < std::abs(r1 - r2) || d < 1e-10) {
        return result; // Не пересекаются
    }
    
    double a = (r1 * r1 - r2 * r2 + d * d) / (2.0 * d);
    double h = std::sqrt(r1 * r1 - a * a);
    
    double px = c1.x() + a * (dx / d);
    double py = c1.y() + a * (dy / d);
    
    result.append(QPointF(px + h * (dy / d), py - h * (dx / d)));
    if (h > 1e-10) {
        result.append(QPointF(px - h * (dy / d), py + h * (dx / d)));
    }
    
    return result;
}

QPointF SnapManager::findPerpendicularToSegment(const QPointF& from, 
                                                 const QPointF& segStart, const QPointF& segEnd)
{
    double dx = segEnd.x() - segStart.x();
    double dy = segEnd.y() - segStart.y();
    double len2 = dx * dx + dy * dy;
    
    if (len2 < 1e-10) return segStart; // Вырожденный отрезок
    
    double t = ((from.x() - segStart.x()) * dx + (from.y() - segStart.y()) * dy) / len2;
    t = std::max(0.0, std::min(1.0, t)); // Ограничиваем [0, 1]
    
    return QPointF(segStart.x() + t * dx, segStart.y() + t * dy);
}

QVector<QPointF> SnapManager::findTangentPointsToCircle(const QPointF& from, 
                                                         const QPointF& center, double radius)
{
    QVector<QPointF> result;
    
    double dx = from.x() - center.x();
    double dy = from.y() - center.y();
    double dist2 = dx * dx + dy * dy;
    double dist = std::sqrt(dist2);
    
    // Точка внутри или на окружности - нет касательной
    if (dist <= radius + 1e-10) return result;
    
    // Угол от центра до from
    double angle = std::atan2(dy, dx);
    
    // Угол между линией и касательной
    double tangentAngle = std::acos(radius / dist);
    
    // Две точки касания
    double a1 = angle + tangentAngle;
    double a2 = angle - tangentAngle;
    
    result.append(QPointF(center.x() + radius * std::cos(a1), 
                          center.y() + radius * std::sin(a1)));
    result.append(QPointF(center.x() + radius * std::cos(a2), 
                          center.y() + radius * std::sin(a2)));
    
    return result;
}

// === СБОР ТОЧЕК ПЕРЕСЕЧЕНИЯ ===

void SnapManager::collectIntersections(const QPointF& mousePos, Scene* scene, 
                                        double tolerance, QVector<SnapPoint>& out)
{
    if (!scene) return;
    
    const auto& primitives = scene->getPrimitives();
    int count = primitives.size();
    
    // Перебираем все пары примитивов
    for (int i = 0; i < count; ++i) {
        for (int j = i + 1; j < count; ++j) {
            auto* p1 = primitives[i].get();
            auto* p2 = primitives[j].get();
            
            QVector<QPointF> intersections;
            
            // Сегмент-Сегмент
            if (p1->getType() == PrimitiveType::Segment && p2->getType() == PrimitiveType::Segment) {
                auto* s1 = static_cast<SegmentPrimitive*>(p1);
                auto* s2 = static_cast<SegmentPrimitive*>(p2);
                intersections = findSegmentSegmentIntersection(
                    QPointF(s1->getStart().getX(), s1->getStart().getY()),
                    QPointF(s1->getEnd().getX(), s1->getEnd().getY()),
                    QPointF(s2->getStart().getX(), s2->getStart().getY()),
                    QPointF(s2->getEnd().getX(), s2->getEnd().getY())
                );
            }
            // Сегмент-Окружность
            else if ((p1->getType() == PrimitiveType::Segment && p2->getType() == PrimitiveType::Circle) ||
                     (p1->getType() == PrimitiveType::Circle && p2->getType() == PrimitiveType::Segment)) {
                SegmentPrimitive* seg = nullptr;
                CirclePrimitive* cir = nullptr;
                if (p1->getType() == PrimitiveType::Segment) {
                    seg = static_cast<SegmentPrimitive*>(p1);
                    cir = static_cast<CirclePrimitive*>(p2);
                } else {
                    seg = static_cast<SegmentPrimitive*>(p2);
                    cir = static_cast<CirclePrimitive*>(p1);
                }
                intersections = findSegmentCircleIntersection(
                    QPointF(seg->getStart().getX(), seg->getStart().getY()),
                    QPointF(seg->getEnd().getX(), seg->getEnd().getY()),
                    QPointF(cir->getCenter().getX(), cir->getCenter().getY()),
                    cir->getRadius()
                );
            }
            // Окружность-Окружность
            else if (p1->getType() == PrimitiveType::Circle && p2->getType() == PrimitiveType::Circle) {
                auto* c1 = static_cast<CirclePrimitive*>(p1);
                auto* c2 = static_cast<CirclePrimitive*>(p2);
                intersections = findCircleCircleIntersection(
                    QPointF(c1->getCenter().getX(), c1->getCenter().getY()), c1->getRadius(),
                    QPointF(c2->getCenter().getX(), c2->getCenter().getY()), c2->getRadius()
                );
            }
            
            // Добавляем найденные точки
            for (const auto& pt : intersections) {
                double dist = QLineF(mousePos, pt).length();
                if (dist <= tolerance) {
                    SnapPoint sp;
                    sp.position = pt;
                    sp.type = SnapType::Intersection;
                    sp.source = p1;
                    sp.distance = dist;
                    out.append(sp);
                }
            }
        }
    }
}

// === СБОР ТОЧЕК ПЕРПЕНДИКУЛЯРА ===

void SnapManager::collectPerpendiculars(const QPointF& mousePos, const QPointF& basePoint,
                                         Scene* scene, double tolerance, QVector<SnapPoint>& out)
{
    if (!scene) return;
    
    for (const auto& primitive : scene->getPrimitives()) {
        auto* prim = primitive.get();
        
        if (prim->getType() == PrimitiveType::Segment) {
            auto* seg = static_cast<SegmentPrimitive*>(prim);
            QPointF perpPt = findPerpendicularToSegment(
                basePoint,
                QPointF(seg->getStart().getX(), seg->getStart().getY()),
                QPointF(seg->getEnd().getX(), seg->getEnd().getY())
            );
            
            double dist = QLineF(mousePos, perpPt).length();
            if (dist <= tolerance) {
                SnapPoint sp;
                sp.position = perpPt;
                sp.type = SnapType::Perpendicular;
                sp.source = prim;
                sp.distance = dist;
                out.append(sp);
            }
        }
        else if (prim->getType() == PrimitiveType::Circle) {
            // Перпендикуляр к окружности = линия через центр
            auto* cir = static_cast<CirclePrimitive*>(prim);
            QPointF center(cir->getCenter().getX(), cir->getCenter().getY());
            double radius = cir->getRadius();
            
            // Направление от basePoint к центру
            double dx = center.x() - basePoint.x();
            double dy = center.y() - basePoint.y();
            double dist = std::sqrt(dx*dx + dy*dy);
            
            if (dist > 1e-10) {
                // Точка на окружности в направлении от basePoint
                QPointF perpPt(center.x() - dx * radius / dist, 
                               center.y() - dy * radius / dist);
                
                double snapDist = QLineF(mousePos, perpPt).length();
                if (snapDist <= tolerance) {
                    SnapPoint sp;
                    sp.position = perpPt;
                    sp.type = SnapType::Perpendicular;
                    sp.source = prim;
                    sp.distance = snapDist;
                    out.append(sp);
                }
            }
        }
    }
}

// === СБОР ТОЧЕК КАСАТЕЛЬНОЙ ===

void SnapManager::collectTangents(const QPointF& mousePos, const QPointF& basePoint,
                                   Scene* scene, double tolerance, QVector<SnapPoint>& out)
{
    if (!scene) return;
    
    for (const auto& primitive : scene->getPrimitives()) {
        auto* prim = primitive.get();
        
        if (prim->getType() == PrimitiveType::Circle) {
            auto* cir = static_cast<CirclePrimitive*>(prim);
            QPointF center(cir->getCenter().getX(), cir->getCenter().getY());
            double radius = cir->getRadius();
            
            QVector<QPointF> tangentPoints = findTangentPointsToCircle(basePoint, center, radius);
            
            for (const auto& pt : tangentPoints) {
                double dist = QLineF(mousePos, pt).length();
                if (dist <= tolerance) {
                    SnapPoint sp;
                    sp.position = pt;
                    sp.type = SnapType::Tangent;
                    sp.source = prim;
                    sp.distance = dist;
                    out.append(sp);
                }
            }
        }
        else if (prim->getType() == PrimitiveType::Arc) {
            auto* arc = static_cast<ArcPrimitive*>(prim);
            QPointF center(arc->getCenter().getX(), arc->getCenter().getY());
            double radius = arc->getRadius();
            
            QVector<QPointF> tangentPoints = findTangentPointsToCircle(basePoint, center, radius);
            
            // Проверяем что точка лежит в пределах дуги
            double startAngle = arc->getStartAngle() * M_PI / 180.0;
            double spanAngle = arc->getSpanAngle() * M_PI / 180.0;
            double endAngle = startAngle + spanAngle;
            
            for (const auto& pt : tangentPoints) {
                double ptAngle = std::atan2(pt.y() - center.y(), pt.x() - center.x());
                // Нормализуем углы
                while (ptAngle < startAngle) ptAngle += 2 * M_PI;
                while (ptAngle > startAngle + 2 * M_PI) ptAngle -= 2 * M_PI;
                
                if (ptAngle >= startAngle && ptAngle <= endAngle) {
                    double dist = QLineF(mousePos, pt).length();
                    if (dist <= tolerance) {
                        SnapPoint sp;
                        sp.position = pt;
                        sp.type = SnapType::Tangent;
                        sp.source = prim;
                        sp.distance = dist;
                        out.append(sp);
                    }
                }
            }
        }
    }
}
