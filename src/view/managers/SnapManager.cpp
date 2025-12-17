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
    // По умолчанию включены: Endpoint, Midpoint, Center, Quadrant, Grid
    m_enabledSnapTypes = static_cast<int>(SnapType::Endpoint) |
                         static_cast<int>(SnapType::Midpoint) |
                         static_cast<int>(SnapType::Center) |
                         static_cast<int>(SnapType::Quadrant) |
                         static_cast<int>(SnapType::Grid);
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
