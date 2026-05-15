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
#include "PointPrimitive.h"

#include <cmath>
#include <algorithm>
#include <limits>
#include <QTransform>

namespace {
QPointF closestPointOnSegment(const QPointF& p, const QPointF& a, const QPointF& b)
{
    const double dx = b.x() - a.x();
    const double dy = b.y() - a.y();
    const double lenSq = dx * dx + dy * dy;
    if (lenSq < 1e-12) return a;
    const double t = std::clamp(((p.x() - a.x()) * dx + (p.y() - a.y()) * dy) / lenSq, 0.0, 1.0);
    return QPointF(a.x() + t * dx, a.y() + t * dy);
}

double normalizeDeg(double angle)
{
    while (angle < 0.0) angle += 360.0;
    while (angle >= 360.0) angle -= 360.0;
    return angle;
}

QPointF pointOnArcQt(const QPointF& center, double radius, double angleDeg)
{
    const double angleRad = angleDeg * M_PI / 180.0;
    return QPointF(center.x() + radius * std::cos(angleRad),
                   center.y() - radius * std::sin(angleRad));
}

bool angleOnArc(double angle, double start, double span)
{
    angle = normalizeDeg(angle);
    start = normalizeDeg(start);
    double rel = angle - start;
    while (rel < 0.0) rel += 360.0;
    if (span >= 0.0) return rel <= span;
    return rel >= (360.0 + span);
}

double snapPriorityFactor(SnapType type)
{
    switch (type) {
    case SnapType::Endpoint:
        return 0.35;
    case SnapType::Midpoint:
        return 0.30;
    case SnapType::Center:
        return 0.35;
    case SnapType::Quadrant:
        return 0.40;
    case SnapType::Nearest:
        return 1.35;
    case SnapType::Grid:
        return 1.10;
    default:
        return 1.0;
    }
}

QVector<QPointF> buildRectanglePolyline(const RectanglePrimitive* rect)
{
    QVector<QPointF> points;
    const QVector<QPointF> snaps = rect->getSnapPoints();
    for (int i = 1; i <= 4 && i < snaps.size(); ++i) {
        points.append(snaps[i]);
    }
    return points;
}

QVector<QPointF> buildEllipsePolyline(const EllipsePrimitive* ellipse, int segments = 180)
{
    QVector<QPointF> points;
    points.reserve(segments);

    const double cx = ellipse->getCenter().getX();
    const double cy = ellipse->getCenter().getY();
    const double rx = ellipse->getRadiusX();
    const double ry = ellipse->getRadiusY();
    const double rotation = ellipse->getRotation() * M_PI / 180.0;
    const double cosA = std::cos(rotation);
    const double sinA = std::sin(rotation);

    for (int i = 0; i < segments; ++i) {
        const double angle = (2.0 * M_PI * i) / segments;
        const double localX = rx * std::cos(angle);
        const double localY = ry * std::sin(angle);
        points.append(QPointF(cx + localX * cosA - localY * sinA,
                              cy + localX * sinA + localY * cosA));
    }

    return points;
}

QVector<QPointF> buildArcPolyline(const ArcPrimitive* arc, int segments = 96)
{
    QVector<QPointF> points;
    const double span = arc->getSpanAngle();
    if (std::abs(span) < 1e-9) {
        return points;
    }

    const int steps = std::max(2, static_cast<int>(std::ceil(std::abs(span) / 360.0 * segments)));
    points.reserve(steps + 1);

    const QPointF center(arc->getCenter().getX(), arc->getCenter().getY());
    for (int i = 0; i <= steps; ++i) {
        const double t = static_cast<double>(i) / steps;
        points.append(pointOnArcQt(center, arc->getRadius(), arc->getStartAngle() + span * t));
    }

    return points;
}

QVector<QLineF> buildPrimitiveSegments(const BasePrimitive* primitive)
{
    QVector<QLineF> segments;
    if (!primitive) {
        return segments;
    }

    auto appendPolyline = [&](const QVector<QPointF>& points, bool closed) {
        if (points.size() < 2) {
            return;
        }
        const int limit = closed ? points.size() : points.size() - 1;
        for (int i = 0; i < limit; ++i) {
            const QPointF& a = points[i];
            const QPointF& b = points[(i + 1) % points.size()];
            if (QLineF(a, b).length() > 1e-9) {
                segments.append(QLineF(a, b));
            }
        }
    };

    switch (primitive->getType()) {
    case PrimitiveType::Segment: {
        auto* seg = static_cast<const SegmentPrimitive*>(primitive);
        segments.append(QLineF(QPointF(seg->getStart().getX(), seg->getStart().getY()),
                               QPointF(seg->getEnd().getX(), seg->getEnd().getY())));
        break;
    }
    case PrimitiveType::Rectangle:
        appendPolyline(buildRectanglePolyline(static_cast<const RectanglePrimitive*>(primitive)), true);
        break;
    case PrimitiveType::Polygon:
        appendPolyline(static_cast<const PolygonPrimitive*>(primitive)->getVertices(), true);
        break;
    case PrimitiveType::Spline:
        appendPolyline(static_cast<const SplinePrimitive*>(primitive)->calculateSplinePoints(), false);
        break;
    case PrimitiveType::Ellipse:
        appendPolyline(buildEllipsePolyline(static_cast<const EllipsePrimitive*>(primitive)), true);
        break;
    case PrimitiveType::Arc:
        appendPolyline(buildArcPolyline(static_cast<const ArcPrimitive*>(primitive)), false);
        break;
    default:
        break;
    }

    return segments;
}

void appendUniqueIntersection(QVector<QPointF>& points, const QPointF& candidate, double eps = 1e-6)
{
    for (const QPointF& existing : points) {
        if (QLineF(existing, candidate).length() <= eps) {
            return;
        }
    }
    points.append(candidate);
}
}

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
                         static_cast<int>(SnapType::Nearest) |
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
            gridSnap.distance = dist * snapPriorityFactor(SnapType::Grid);
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
    if (isSnapTypeEnabled(SnapType::Nearest)) {
        collectNearestPoints(mousePos, primitive, outPoints);
    }
    
    // Расчет расстояний и фильтрация по толерансу
    for (int i = outPoints.size() - 1; i >= 0; --i) {
        const double rawDistance = QLineF(mousePos, outPoints[i].position).length();
        if (rawDistance > tolerance) {
            outPoints.removeAt(i);
            continue;
        }
        outPoints[i].distance = rawDistance * snapPriorityFactor(outPoints[i].type);
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
        const QPointF center(arc->getCenter().getX(), arc->getCenter().getY());
        sp.position = pointOnArcQt(center, arc->getRadius(), arc->getStartAngle());
        out.append(sp);
        sp.position = pointOnArcQt(center, arc->getRadius(), arc->getStartAngle() + arc->getSpanAngle());
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
        const QPointF center(arc->getCenter().getX(), arc->getCenter().getY());
        sp.position = pointOnArcQt(center, arc->getRadius(), arc->getStartAngle() + arc->getSpanAngle() / 2.0);
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

void SnapManager::collectNearestPoints(const QPointF& mousePos, BasePrimitive* prim, QVector<SnapPoint>& out)
{
    SnapPoint sp;
    sp.type = SnapType::Nearest;
    sp.source = prim;

    switch (prim->getType()) {
    case PrimitiveType::Segment: {
        auto* seg = static_cast<SegmentPrimitive*>(prim);
        QPointF a(seg->getStart().getX(), seg->getStart().getY());
        QPointF b(seg->getEnd().getX(), seg->getEnd().getY());
        sp.position = closestPointOnSegment(mousePos, a, b);
        out.append(sp);
        break;
    }
    case PrimitiveType::Circle: {
        auto* circle = static_cast<CirclePrimitive*>(prim);
        QPointF center(circle->getCenter().getX(), circle->getCenter().getY());
        QLineF line(center, mousePos);
        if (line.length() < 1e-6) break;
        line.setLength(circle->getRadius());
        sp.position = line.p2();
        out.append(sp);
        break;
    }
    case PrimitiveType::Arc: {
        auto* arc = static_cast<ArcPrimitive*>(prim);
        QPointF center(arc->getCenter().getX(), arc->getCenter().getY());
        double angle = normalizeDeg(QLineF(center, mousePos).angle());
        if (!angleOnArc(angle, arc->getStartAngle(), arc->getSpanAngle())) {
            QPointF start = pointOnArcQt(center, arc->getRadius(), arc->getStartAngle());
            QPointF end = pointOnArcQt(center, arc->getRadius(), arc->getStartAngle() + arc->getSpanAngle());
            sp.position = QLineF(mousePos, start).length() < QLineF(mousePos, end).length() ? start : end;
        } else {
            sp.position = pointOnArcQt(center, arc->getRadius(), angle);
        }
        out.append(sp);
        break;
    }
    case PrimitiveType::Ellipse: {
        auto* ellipse = static_cast<EllipsePrimitive*>(prim);
        QTransform t;
        t.translate(ellipse->getCenter().getX(), ellipse->getCenter().getY());
        t.rotate(ellipse->getRotation());
        QTransform inv = t.inverted();
        QPointF local = inv.map(mousePos);
        double angle = std::atan2(local.y() * ellipse->getRadiusX(), local.x() * ellipse->getRadiusY());
        QPointF localOnEllipse(ellipse->getRadiusX() * std::cos(angle), ellipse->getRadiusY() * std::sin(angle));
        sp.position = t.map(localOnEllipse);
        out.append(sp);
        break;
    }
    case PrimitiveType::Rectangle: {
        auto* rect = static_cast<RectanglePrimitive*>(prim);
        QPolygonF poly = rect->getBoundingBox().isValid() ? QPolygonF(rect->getBoundingBox()) : QPolygonF();
        poly = rect->getSnapPoints().mid(1, 4);
        double best = std::numeric_limits<double>::max();
        QPointF bestPoint = mousePos;
        for (int i = 0; i < poly.size(); ++i) {
            QPointF p = closestPointOnSegment(mousePos, poly[i], poly[(i + 1) % poly.size()]);
            double d = QLineF(mousePos, p).length();
            if (d < best) { best = d; bestPoint = p; }
        }
        sp.position = bestPoint;
        out.append(sp);
        break;
    }
    case PrimitiveType::Polygon: {
        auto* poly = static_cast<PolygonPrimitive*>(prim);
        QVector<QPointF> vertices = poly->getVertices();
        double best = std::numeric_limits<double>::max();
        QPointF bestPoint = mousePos;
        for (int i = 0; i < vertices.size(); ++i) {
            QPointF p = closestPointOnSegment(mousePos, vertices[i], vertices[(i + 1) % vertices.size()]);
            double d = QLineF(mousePos, p).length();
            if (d < best) { best = d; bestPoint = p; }
        }
        sp.position = bestPoint;
        out.append(sp);
        break;
    }
    case PrimitiveType::Spline: {
        auto* spline = static_cast<SplinePrimitive*>(prim);
        QVector<QPointF> pts = spline->getControlPoints();
        double best = std::numeric_limits<double>::max();
        QPointF bestPoint = mousePos;
        for (int i = 0; i < pts.size() - 1; ++i) {
            QPointF p = closestPointOnSegment(mousePos, pts[i], pts[i + 1]);
            double d = QLineF(mousePos, p).length();
            if (d < best) { best = d; bestPoint = p; }
        }
        sp.position = bestPoint;
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

QVector<QPointF> SnapManager::findTangentPointsToEllipse(const QPointF& from,
                                                         const QPointF& center, double rx, double ry, double rotation)
{
    QVector<QPointF> result;
    
    // Переводим точку from в локальную систему координат эллипса
    double dx = from.x() - center.x();
    double dy = from.y() - center.y();
    double rotRad = -rotation; // Обратный поворот
    
    double lx = dx * std::cos(rotRad) - dy * std::sin(rotRad);
    double ly = dx * std::sin(rotRad) + dy * std::cos(rotRad);
    
    // Если точка внутри эллипса lx^2/rx^2 + ly^2/ry^2 < 1, касательных нет
    if ((lx*lx)/(rx*rx) + (ly*ly)/(ry*ry) <= 1.0 + 1e-9) {
        return result;
    }

    // Используем метод Ньютона для поиска параметра t
    // Уравнение нормали: x(t) = rx*cos(t), y(t) = ry*sin(t)
    // Вектор касательной: (-rx*sin(t), ry*cos(t))
    // Вектор от точки к касательной: (radiusX*cos(t) - lx, radiusY*sin(t) - ly)
    // Они должны быть коллинеарны -> векторное произведение = 0
    // f(t) = (rx*cos(t) - lx) * ry*cos(t) - (ry*sin(t) - ly) * (-rx*sin(t)) = 0
    // f(t) = rx*ry*cos^2(t) - lx*ry*cos(t) + rx*ry*sin^2(t) - ly*rx*sin(t)
    // f(t) = rx*ry - lx*ry*cos(t) - ly*rx*sin(t)
    // Нам нужно найти нули функции f(t)
    
    auto f = [&](double t) {
        return rx * ry - lx * ry * std::cos(t) - ly * rx * std::sin(t);
    };
    
    auto df = [&](double t) {
        return lx * ry * std::sin(t) - ly * rx * std::cos(t);
    };
    
    // Начальные приближения. Поскольку касательных обычно две, 
    // попробуем найти их, стартуя с разных углов.
    double initialGuesses[] = {0, M_PI_2, M_PI, 3.0 * M_PI_2};
    std::vector<double> solutions;

    for (double t0 : initialGuesses) {
        double t = t0;
        for (int i = 0; i < 10; ++i) {
            double val = f(t);
            double dval = df(t);
            if (std::abs(dval) < 1e-9) break;
            double nextT = t - val / dval;
            if (std::abs(nextT - t) < 1e-9) {
                t = nextT;
                // Нормализация t
                while(t < 0) t += 2*M_PI;
                while(t >= 2*M_PI) t -= 2*M_PI;
                
                // Проверяем, уникально ли решение
                bool exists = false;
                for (double s : solutions) {
                    if (std::abs(s - t) < 1e-4 || std::abs(s - t - 2*M_PI) < 1e-4 || std::abs(s - t + 2*M_PI) < 1e-4) {
                        exists = true;
                        break;
                    }
                }
                if (!exists) solutions.push_back(t);
                break;
            }
            t = nextT;
        }
    }

    for (double t : solutions) {
        // Точка на эллипсе в локальных координатах
        double ex = rx * std::cos(t);
        double ey = ry * std::sin(t);
        
        // Обратно в мировые координаты
        double worldX = center.x() + ex * std::cos(rotation) - ey * std::sin(rotation);
        double worldY = center.y() + ex * std::sin(rotation) + ey * std::cos(rotation);
        
        result.append(QPointF(worldX, worldY));
    }
    
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
            else {
                const bool p1Circle = p1->getType() == PrimitiveType::Circle;
                const bool p2Circle = p2->getType() == PrimitiveType::Circle;

                if (p1Circle || p2Circle) {
                    CirclePrimitive* circle = p1Circle ? static_cast<CirclePrimitive*>(p1)
                                                       : static_cast<CirclePrimitive*>(p2);
                    BasePrimitive* other = p1Circle ? p2 : p1;
                    const QVector<QLineF> otherSegments = buildPrimitiveSegments(other);
                    const QPointF center(circle->getCenter().getX(), circle->getCenter().getY());

                    for (const QLineF& segment : otherSegments) {
                        const QVector<QPointF> hits = findSegmentCircleIntersection(segment.p1(), segment.p2(),
                                                                                    center, circle->getRadius());
                        for (const QPointF& hit : hits) {
                            appendUniqueIntersection(intersections, hit);
                        }
                    }
                } else {
                    const QVector<QLineF> firstSegments = buildPrimitiveSegments(p1);
                    const QVector<QLineF> secondSegments = buildPrimitiveSegments(p2);

                    for (const QLineF& s1 : firstSegments) {
                        for (const QLineF& s2 : secondSegments) {
                            const QVector<QPointF> hits = findSegmentSegmentIntersection(
                                s1.p1(), s1.p2(), s2.p1(), s2.p2());
                            for (const QPointF& hit : hits) {
                                appendUniqueIntersection(intersections, hit);
                            }
                        }
                    }
                }
            }
            
            // Добавляем найденные точки
            for (const auto& pt : intersections) {
                double dist = QLineF(mousePos, pt).length();
                if (dist <= tolerance) {
                    SnapPoint sp;
                    sp.position = pt;
                    sp.type = SnapType::Intersection;
                    sp.source = p1;
                    sp.distance = dist * 0.2;
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
    
    auto checkAndAdd = [&](const QPointF& perpPt, BasePrimitive* prim, double tVal_optional = -1.0) {
        // Если указано tVal, проверяем на строгое попадание внутрь отрезка
        // чтобы не конфликтовать с Endpoint snap (t=0 или t=1)
        if (tVal_optional >= 0.0) {
            if (tVal_optional < 0.01 || tVal_optional > 0.99) return;
        }

        double dist = QLineF(mousePos, perpPt).length();
        if (dist <= tolerance) {
            SnapPoint sp;
            sp.position = perpPt;
            sp.type = SnapType::Perpendicular;
            sp.source = prim;
            // Перпендикуляр в CAD обычно имеет высокий приоритет, если курсор уже рядом с его
            // геометрически корректной точкой.
            sp.distance = dist * 0.25;
            out.append(sp);
        }
    };

    for (const auto& primitive : scene->getPrimitives()) {
        auto* prim = primitive.get();
        
        if (prim->getType() == PrimitiveType::Segment) {
            auto* seg = static_cast<SegmentPrimitive*>(prim);
            double dx = seg->getEnd().getX() - seg->getStart().getX();
            double dy = seg->getEnd().getY() - seg->getStart().getY();
            double len2 = dx * dx + dy * dy;
            
            if (len2 > 1e-10) {
                 double t = ((basePoint.x() - seg->getStart().getX()) * dx + (basePoint.y() - seg->getStart().getY()) * dy) / len2;
                 QPointF perpPt(seg->getStart().getX() + t * dx, seg->getStart().getY() + t * dy);
                 
                 // t должен быть в пределах сегмента для перпендикуляра _к сегменту_
                 if (t >= 0.0 && t <= 1.0) {
                     checkAndAdd(perpPt, prim, t);
                 }
            }
        }
        else if (prim->getType() == PrimitiveType::Circle) {
            auto* cir = static_cast<CirclePrimitive*>(prim);
            QPointF center(cir->getCenter().getX(), cir->getCenter().getY());
            double radius = cir->getRadius();
            
            double dx = center.x() - basePoint.x();
            double dy = center.y() - basePoint.y();
            double dist = std::sqrt(dx*dx + dy*dy);
            
            if (dist > 1e-10) {
                QPointF perpPt(center.x() - dx * radius / dist, 
                               center.y() - dy * radius / dist);
                checkAndAdd(perpPt, prim);
            }
        }
        else if (prim->getType() == PrimitiveType::Arc) {
            auto* arc = static_cast<ArcPrimitive*>(prim);
            QPointF center(arc->getCenter().getX(), arc->getCenter().getY());
            const double radius = arc->getRadius();
            const double dx = center.x() - basePoint.x();
            const double dy = center.y() - basePoint.y();
            const double dist = std::sqrt(dx * dx + dy * dy);

            if (dist > 1e-10) {
                QPointF perpPt(center.x() - dx * radius / dist,
                               center.y() - dy * radius / dist);
                const double angle = normalizeDeg(QLineF(center, perpPt).angle());
                if (angleOnArc(angle, arc->getStartAngle(), arc->getSpanAngle())) {
                    checkAndAdd(perpPt, prim);
                }
            }
        }
        else if (prim->getType() == PrimitiveType::Ellipse) {
            auto* ellipse = static_cast<EllipsePrimitive*>(prim);
            QTransform t;
            t.translate(ellipse->getCenter().getX(), ellipse->getCenter().getY());
            t.rotate(ellipse->getRotation());

            double bestDistance = std::numeric_limits<double>::max();
            QPointF bestPoint;
            bool found = false;
            const int segments = 180;

            QPointF prev = t.map(QPointF(ellipse->getRadiusX(), 0.0));
            for (int i = 1; i <= segments; ++i) {
                const double a = (2.0 * M_PI * i) / segments;
                const QPointF curr = t.map(QPointF(ellipse->getRadiusX() * std::cos(a),
                                                   ellipse->getRadiusY() * std::sin(a)));
                const QPointF candidate = closestPointOnSegment(basePoint, prev, curr);
                const double d = QLineF(basePoint, candidate).length();
                if (d < bestDistance) {
                    bestDistance = d;
                    bestPoint = candidate;
                    found = true;
                }
                prev = curr;
            }

            if (found) {
                checkAndAdd(bestPoint, prim);
            }
        }
        else if (prim->getType() == PrimitiveType::Rectangle) {
            auto* rect = static_cast<RectanglePrimitive*>(prim);
            
            double cx = rect->getCenter().getX();
            double cy = rect->getCenter().getY();
            double hw = rect->getWidth() / 2.0;
            double hh = rect->getHeight() / 2.0;
            double angle = rect->getRotation() * M_PI / 180.0;
            
            double cosA = std::cos(angle);
            double sinA = std::sin(angle);
            
            QVector<QPointF> corners;
            corners << QPointF(cx + (-hw) * cosA - (-hh) * sinA, cy + (-hw) * sinA + (-hh) * cosA);
            corners << QPointF(cx + ( hw) * cosA - (-hh) * sinA, cy + ( hw) * sinA + (-hh) * cosA);
            corners << QPointF(cx + ( hw) * cosA - ( hh) * sinA, cy + ( hw) * sinA + ( hh) * cosA);
            corners << QPointF(cx + (-hw) * cosA - ( hh) * sinA, cy + (-hw) * sinA + ( hh) * cosA);
            
            for (int i = 0; i < 4; ++i) {
                QPointF p1 = corners[i];
                QPointF p2 = corners[(i + 1) % 4];
                
                double dx = p2.x() - p1.x();
                double dy = p2.y() - p1.y();
                double len2 = dx * dx + dy * dy;

                if (len2 > 1e-10) {
                    double t = ((basePoint.x() - p1.x()) * dx + (basePoint.y() - p1.y()) * dy) / len2;
                    if (t >= 0.0 && t <= 1.0) {
                        QPointF perpPt(p1.x() + t * dx, p1.y() + t * dy);
                        checkAndAdd(perpPt, prim, t);
                    }
                }
            }
        }
        else if (prim->getType() == PrimitiveType::Polygon) {
            auto* poly = static_cast<PolygonPrimitive*>(prim);
            QVector<QPointF> vertices = poly->getVertices();
            
            for (int i = 0; i < vertices.size(); ++i) {
                QPointF p1 = vertices[i];
                QPointF p2 = vertices[(i + 1) % vertices.size()];
                
                double dx = p2.x() - p1.x();
                double dy = p2.y() - p1.y();
                double len2 = dx * dx + dy * dy;

                if (len2 > 1e-10) {
                    double t = ((basePoint.x() - p1.x()) * dx + (basePoint.y() - p1.y()) * dy) / len2;
                    if (t >= 0.0 && t <= 1.0) {
                        QPointF perpPt(p1.x() + t * dx, p1.y() + t * dy);
                        checkAndAdd(perpPt, prim, t);
                    }
                }
            }
        }
        else if (prim->getType() == PrimitiveType::Spline) {
            auto* spline = static_cast<SplinePrimitive*>(prim);
            const QVector<QPointF> points = spline->calculateSplinePoints();

            for (int i = 0; i + 1 < points.size(); ++i) {
                const QPointF& p1 = points[i];
                const QPointF& p2 = points[i + 1];
                const double dx = p2.x() - p1.x();
                const double dy = p2.y() - p1.y();
                const double len2 = dx * dx + dy * dy;

                if (len2 > 1e-10) {
                    const double t = ((basePoint.x() - p1.x()) * dx + (basePoint.y() - p1.y()) * dy) / len2;
                    if (t >= 0.0 && t <= 1.0) {
                        const QPointF perpPt(p1.x() + t * dx, p1.y() + t * dy);
                        checkAndAdd(perpPt, prim, t);
                    }
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
    
    auto processTangentPoint = [&](QPointF pt, BasePrimitive* prim) {
        // Касательная должна срабатывать, когда курсор ведется вдоль касательного луча,
        // но конечная точка построения остается именно в точке касания.
        const QPointF direction = pt - basePoint;
        const double lineLen2 = QPointF::dotProduct(direction, direction);
        if (lineLen2 < 1e-10) return;

        const QPointF toMouse = mousePos - basePoint;
        const double t = QPointF::dotProduct(toMouse, direction) / lineLen2;

        // Не разрешаем касательную "назад" за базовую точку.
        if (t < 0.0) return;

        const QPointF projectedPoint = basePoint + t * direction;
        const double rayDistance = QLineF(mousePos, projectedPoint).length();
        if (rayDistance > tolerance) return;

        SnapPoint sp;
        sp.position = pt;
        sp.type = SnapType::Tangent;
        sp.source = prim;
        sp.distance = rayDistance;
        out.append(sp);
    };

    for (const auto& primitive : scene->getPrimitives()) {
        auto* prim = primitive.get();
        
        if (prim->getType() == PrimitiveType::Circle) {
            auto* cir = static_cast<CirclePrimitive*>(prim);
            QPointF center(cir->getCenter().getX(), cir->getCenter().getY());
            double radius = cir->getRadius();
            
            QVector<QPointF> tangentPoints = findTangentPointsToCircle(basePoint, center, radius);
            for (const auto& pt : tangentPoints) {
                processTangentPoint(pt, prim);
            }
        }
        else if (prim->getType() == PrimitiveType::Arc) {
            auto* arc = static_cast<ArcPrimitive*>(prim);
            QPointF center(arc->getCenter().getX(), arc->getCenter().getY());
            double radius = arc->getRadius();
            
            QVector<QPointF> tangentPoints = findTangentPointsToCircle(basePoint, center, radius);

            for (const auto& pt : tangentPoints) {
                 if (angleOnArc(QLineF(center, pt).angle(), arc->getStartAngle(), arc->getSpanAngle())) {
                    processTangentPoint(pt, prim);
                }
            }
        }
        else if (prim->getType() == PrimitiveType::Ellipse) {
            auto* ellipse = static_cast<EllipsePrimitive*>(prim);
            QPointF center(ellipse->getCenter().getX(), ellipse->getCenter().getY());
            
            QVector<QPointF> tangentPoints = findTangentPointsToEllipse(
                basePoint, center, 
                ellipse->getRadiusX(), ellipse->getRadiusY(), 
                ellipse->getRotation() * M_PI / 180.0
            );
            
            for (const auto& pt : tangentPoints) {
                processTangentPoint(pt, prim);
            }
        }
    }
}
