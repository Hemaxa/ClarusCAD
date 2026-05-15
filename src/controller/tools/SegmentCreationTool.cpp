#include "SegmentCreationTool.h"
#include "Scene.h"
#include "ViewportPanelWidget.h"
#include "LineStyleManager.h"
#include "ArcPrimitive.h"
#include "CirclePrimitive.h"
#include "EllipsePrimitive.h"
#include "PolygonPrimitive.h"
#include "RectanglePrimitive.h"
#include "SegmentPrimitive.h"
#include "SplinePrimitive.h"

#include <QMouseEvent>
#include <QPainter>
#include <QPolygonF>
#include <algorithm>
#include <cmath>
#include <limits>

namespace {
constexpr double kGeomEps = 1e-9;

enum class OrthoAxis {
    Horizontal,
    Vertical
};

double normalizeDeg(double angle)
{
    while (angle < 0.0) angle += 360.0;
    while (angle >= 360.0) angle -= 360.0;
    return angle;
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

OrthoAxis detectAxis(const QPointF& firstPoint, const QPointF& orthoPoint)
{
    if (std::abs(orthoPoint.y() - firstPoint.y()) <= std::abs(orthoPoint.x() - firstPoint.x())) {
        return OrthoAxis::Horizontal;
    }
    return OrthoAxis::Vertical;
}

double axisValue(const QPointF& point, OrthoAxis axis)
{
    return axis == OrthoAxis::Horizontal ? point.x() : point.y();
}

double axisDistanceFromAnchor(const QPointF& point, const QPointF& anchor, OrthoAxis axis)
{
    return axisValue(point, axis) - axisValue(anchor, axis);
}

bool isPointOnAxis(const QPointF& point, const QPointF& anchor, OrthoAxis axis, double tolerance)
{
    if (axis == OrthoAxis::Horizontal) {
        return std::abs(point.y() - anchor.y()) <= tolerance;
    }
    return std::abs(point.x() - anchor.x()) <= tolerance;
}

QPointF clampPointToAxis(const QPointF& point, const QPointF& anchor, OrthoAxis axis)
{
    QPointF result = point;
    if (axis == OrthoAxis::Horizontal) {
        result.setY(anchor.y());
    } else {
        result.setX(anchor.x());
    }
    return result;
}

bool isCandidateAlongRay(const QPointF& point, const QPointF& anchor, const QPointF& cursorPoint,
                         OrthoAxis axis, double tolerance)
{
    const double target = axisDistanceFromAnchor(cursorPoint, anchor, axis);
    const double candidate = axisDistanceFromAnchor(point, anchor, axis);

    if (std::abs(target) <= tolerance) {
        return std::abs(candidate) <= tolerance;
    }

    const double dir = target >= 0.0 ? 1.0 : -1.0;
    const double targetForward = dir * target;
    const double candidateForward = dir * candidate;
    return candidateForward >= -tolerance && candidateForward <= targetForward + tolerance;
}

void appendUniquePoint(QVector<QPointF>& out, const QPointF& point, double tolerance = 1e-6)
{
    for (const QPointF& existing : out) {
        if (QLineF(existing, point).length() <= tolerance) {
            return;
        }
    }
    out.append(point);
}

QVector<QPointF> intersectAxisWithSegment(const QPointF& a, const QPointF& b, const QPointF& anchor, OrthoAxis axis)
{
    QVector<QPointF> result;

    if (axis == OrthoAxis::Horizontal) {
        const double y = anchor.y();
        const double dy = b.y() - a.y();
        if (std::abs(dy) < kGeomEps) {
            return result;
        }
        const double t = (y - a.y()) / dy;
        if (t >= -kGeomEps && t <= 1.0 + kGeomEps) {
            result.append(QPointF(a.x() + (b.x() - a.x()) * t, y));
        }
        return result;
    }

    const double x = anchor.x();
    const double dx = b.x() - a.x();
    if (std::abs(dx) < kGeomEps) {
        return result;
    }
    const double t = (x - a.x()) / dx;
    if (t >= -kGeomEps && t <= 1.0 + kGeomEps) {
        result.append(QPointF(x, a.y() + (b.y() - a.y()) * t));
    }
    return result;
}

QVector<QPointF> intersectAxisWithCircle(const QPointF& center, double radius,
                                         const QPointF& anchor, OrthoAxis axis)
{
    QVector<QPointF> result;

    if (axis == OrthoAxis::Horizontal) {
        const double dy = anchor.y() - center.y();
        const double disc = radius * radius - dy * dy;
        if (disc < -kGeomEps) {
            return result;
        }
        const double dx = std::sqrt(std::max(0.0, disc));
        appendUniquePoint(result, QPointF(center.x() - dx, anchor.y()));
        appendUniquePoint(result, QPointF(center.x() + dx, anchor.y()));
        return result;
    }

    const double dx = anchor.x() - center.x();
    const double disc = radius * radius - dx * dx;
    if (disc < -kGeomEps) {
        return result;
    }
    const double dy = std::sqrt(std::max(0.0, disc));
    appendUniquePoint(result, QPointF(anchor.x(), center.y() - dy));
    appendUniquePoint(result, QPointF(anchor.x(), center.y() + dy));
    return result;
}

double qtAngleForPoint(const QPointF& center, const QPointF& point)
{
    return normalizeDeg(QLineF(center, point).angle());
}

QVector<QPointF> intersectAxisWithArc(const ArcPrimitive* arc, const QPointF& anchor, OrthoAxis axis)
{
    QVector<QPointF> result;
    const QPointF center(arc->getCenter().getX(), arc->getCenter().getY());
    const QVector<QPointF> circleHits = intersectAxisWithCircle(center, arc->getRadius(), anchor, axis);

    for (const QPointF& hit : circleHits) {
        const double angle = qtAngleForPoint(center, hit);
        if (angleOnArc(angle, arc->getStartAngle(), arc->getSpanAngle())) {
            appendUniquePoint(result, hit);
        }
    }

    return result;
}

QVector<QPointF> intersectAxisWithPolyline(const QVector<QPointF>& points, const QPointF& anchor, OrthoAxis axis,
                                           bool closed)
{
    QVector<QPointF> result;
    if (points.size() < 2) {
        return result;
    }

    const int limit = closed ? points.size() : points.size() - 1;
    for (int i = 0; i < limit; ++i) {
        const QPointF& a = points[i];
        const QPointF& b = points[(i + 1) % points.size()];
        const QVector<QPointF> hits = intersectAxisWithSegment(a, b, anchor, axis);
        for (const QPointF& hit : hits) {
            appendUniquePoint(result, hit);
        }
    }
    return result;
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

QVector<QPointF> collectConstraintIntersections(Scene* scene, const QPointF& anchor, const QPointF& cursorPoint,
                                                OrthoAxis axis)
{
    QVector<QPointF> result;
    if (!scene) {
        return result;
    }

    for (const auto& primitive : scene->getPrimitives()) {
        BasePrimitive* prim = primitive.get();
        QVector<QPointF> hits;

        switch (prim->getType()) {
        case PrimitiveType::Segment: {
            auto* segment = static_cast<SegmentPrimitive*>(prim);
            hits = intersectAxisWithSegment(QPointF(segment->getStart().getX(), segment->getStart().getY()),
                                            QPointF(segment->getEnd().getX(), segment->getEnd().getY()),
                                            anchor, axis);
            break;
        }
        case PrimitiveType::Circle: {
            auto* circle = static_cast<CirclePrimitive*>(prim);
            hits = intersectAxisWithCircle(QPointF(circle->getCenter().getX(), circle->getCenter().getY()),
                                           circle->getRadius(), anchor, axis);
            break;
        }
        case PrimitiveType::Arc: {
            hits = intersectAxisWithArc(static_cast<ArcPrimitive*>(prim), anchor, axis);
            break;
        }
        case PrimitiveType::Rectangle: {
            auto* rectangle = static_cast<RectanglePrimitive*>(prim);
            const QVector<QPointF> snapPoints = rectangle->getSnapPoints();
            QVector<QPointF> corners;
            for (int i = 1; i <= 4 && i < snapPoints.size(); ++i) {
                corners.append(snapPoints[i]);
            }
            hits = intersectAxisWithPolyline(corners, anchor, axis, true);
            break;
        }
        case PrimitiveType::Polygon: {
            auto* polygon = static_cast<PolygonPrimitive*>(prim);
            hits = intersectAxisWithPolyline(polygon->getVertices(), anchor, axis, true);
            break;
        }
        case PrimitiveType::Spline: {
            auto* spline = static_cast<SplinePrimitive*>(prim);
            hits = intersectAxisWithPolyline(spline->calculateSplinePoints(), anchor, axis, false);
            break;
        }
        case PrimitiveType::Ellipse: {
            auto* ellipse = static_cast<EllipsePrimitive*>(prim);
            hits = intersectAxisWithPolyline(buildEllipsePolyline(ellipse), anchor, axis, true);
            break;
        }
        default:
            break;
        }

        for (const QPointF& hit : hits) {
            if (QLineF(hit, anchor).length() <= 1e-6) {
                continue;
            }
            if (isCandidateAlongRay(hit, anchor, cursorPoint, axis, 1e-6)) {
                appendUniquePoint(result, clampPointToAxis(hit, anchor, axis));
            }
        }
    }

    std::sort(result.begin(), result.end(), [&](const QPointF& lhs, const QPointF& rhs) {
        const double lhsForward = std::abs(axisDistanceFromAnchor(lhs, anchor, axis));
        const double rhsForward = std::abs(axisDistanceFromAnchor(rhs, anchor, axis));
        return lhsForward < rhsForward;
    });

    return result;
}

QVector<SnapPoint> collectAxisAlignedSnapCandidates(Scene* scene, const QPointF& anchor, const QPointF& cursorPoint,
                                                    OrthoAxis axis, ViewportPanelWidget* viewport)
{
    QVector<SnapPoint> candidates;
    if (!scene || !viewport) {
        return candidates;
    }

    const double zoom = std::max(1e-6, viewport->getZoomFactor());
    const double worldTolerance = 15.0 / zoom;
    const QVector<SnapPoint> nearby = SnapManager::instance().findSnapPointsInRadius(
        cursorPoint, scene, zoom, 15.0);

    for (SnapPoint candidate : nearby) {
        if (candidate.type == SnapType::None || candidate.type == SnapType::Nearest) {
            continue;
        }
        if (!isPointOnAxis(candidate.position, anchor, axis, worldTolerance)) {
            continue;
        }
        candidate.position = clampPointToAxis(candidate.position, anchor, axis);
        if (!isCandidateAlongRay(candidate.position, anchor, cursorPoint, axis, worldTolerance)) {
            continue;
        }
        candidates.append(candidate);
    }

    return candidates;
}
}

//вызывается конструктор базового класса и устанавливается начальное состояние в Idle (покой)
SegmentCreationTool::SegmentCreationTool(QObject* parent) : BaseCreationTool(parent), m_currentState(State::Idle) {}

QPointF SegmentCreationTool::constrainToOrthoAxis(const QPointF& worldPos) const
{
    const QPointF firstPt(m_firstPoint.getX(), m_firstPoint.getY());
    const QPointF delta = worldPos - firstPt;

    if (std::abs(delta.x()) >= std::abs(delta.y())) {
        return QPointF(worldPos.x(), firstPt.y());
    }
    return QPointF(firstPt.x(), worldPos.y());
}

SegmentCreationTool::ResolvedSnap SegmentCreationTool::resolveSecondPoint(const QPointF& rawWorldPos,
                                                                          Qt::KeyboardModifiers modifiers,
                                                                          Scene* scene,
                                                                          ViewportPanelWidget* viewport) const
{
    ResolvedSnap resolved { rawWorldPos, SnapType::None };

    if (!viewport) {
        return resolved;
    }

    if (!(modifiers & Qt::ShiftModifier) || m_currentState != State::WaitingForSecondPoint) {
        const SnapPoint snap = viewport->getSnapPoint(rawWorldPos);
        resolved.position = snap.position;
        resolved.type = snap.type;

        if (snap.type == SnapType::Tangent && m_currentState == State::WaitingForSecondPoint) {
            const QPointF firstPt(m_firstPoint.getX(), m_firstPoint.getY());
            const QPointF tangentDir = snap.position - firstPt;
            const double tangentLen2 = QPointF::dotProduct(tangentDir, tangentDir);
            if (tangentLen2 > 1e-10) {
                const QPointF toCursor = rawWorldPos - firstPt;
                const double t = QPointF::dotProduct(toCursor, tangentDir) / tangentLen2;
                if (t > 1.0) {
                    resolved.position = firstPt + tangentDir * t;
                }
            }
        }
        return resolved;
    }

    const QPointF firstPt(m_firstPoint.getX(), m_firstPoint.getY());
    const QPointF orthoCandidate = constrainToOrthoAxis(rawWorldPos);
    const OrthoAxis axis = detectAxis(firstPt, orthoCandidate);
    const double worldTolerance = 15.0 / std::max(1e-6, viewport->getZoomFactor());

    const QVector<SnapPoint> genericCandidates =
        collectAxisAlignedSnapCandidates(scene, firstPt, orthoCandidate, axis, viewport);

    if (!genericCandidates.isEmpty()) {
        auto bestIt = std::min_element(genericCandidates.begin(), genericCandidates.end(),
                                       [](const SnapPoint& lhs, const SnapPoint& rhs) {
                                           return lhs.distance < rhs.distance;
                                       });
        resolved.position = bestIt->position;
        resolved.type = bestIt->type;
        return resolved;
    }

    const QVector<QPointF> intersections = collectConstraintIntersections(scene, firstPt, orthoCandidate, axis);
    if (!intersections.isEmpty()) {
        resolved.position = intersections.first();
        resolved.type = SnapType::Intersection;
        return resolved;
    }

    resolved.position = clampPointToAxis(orthoCandidate, firstPt, axis);
    if (std::abs(axisDistanceFromAnchor(resolved.position, firstPt, axis)) <= worldTolerance) {
        resolved.type = SnapType::None;
    }
    return resolved;
}

void SegmentCreationTool::onMousePress(QMouseEvent* event, Scene* scene, ViewportPanelWidget* viewport)
{
    //создание нового примитива
    if (event->button() == Qt::LeftButton) {
        //определение координат, в зависимости от активации опции "Привязка к сетке"
        const ResolvedSnap resolved = resolveSecondPoint(event->position(), event->modifiers(), scene, viewport);
        const QPointF snappedPos = resolved.position;
        if (viewport) {
            viewport->setPreviewSnapPoint(SnapPoint{ snappedPos, resolved.type, nullptr, 0.0 });
        }

        //первый клик
        if (m_currentState == State::Idle) {
            m_firstPoint.setX(snappedPos.x());
            m_firstPoint.setY(snappedPos.y());
            m_currentMousePos = m_firstPoint;
            m_currentState = State::WaitingForSecondPoint;
            
            // Устанавливаем базовую точку для привязок перпендикуляр/касательная
            SnapManager::instance().setBasePoint(snappedPos);
        }
        //второй клик
        else if (m_currentState == State::WaitingForSecondPoint) {
            PointPrimitive secondPoint(snappedPos.x(), snappedPos.y());
            emit segmentDataReady(nullptr, m_firstPoint, secondPoint, m_currentColor, m_currentLineType);
            m_currentState = State::Idle;
            
            // Очищаем базовую точку
            SnapManager::instance().clearBasePoint();
        }
    }
    //отмена создания
    else if (event->button() == Qt::RightButton) {
        m_currentState = State::Idle;
        SnapManager::instance().clearBasePoint();
        if (viewport) {
            viewport->setPreviewSnapPoint(SnapPoint{ event->position(), SnapType::None, nullptr, 0.0 });
        }
    }
}

void SegmentCreationTool::onMouseMove(QMouseEvent* event, Scene* scene, ViewportPanelWidget* viewport)
{
    //приклеивание к сетке, если включена опция "Привязка к сетке"
    if (m_currentState == State::WaitingForSecondPoint) {
        const ResolvedSnap resolved = resolveSecondPoint(event->position(), event->modifiers(), scene, viewport);
        const QPointF snappedPos = resolved.position;
        m_currentMousePos.setX(snappedPos.x());
        m_currentMousePos.setY(snappedPos.y());
        if (viewport) {
            viewport->setPreviewSnapPoint(SnapPoint{ snappedPos, resolved.type, nullptr, 0.0 });
        }
    }
}

void SegmentCreationTool::onMouseRelease(QMouseEvent* event, Scene* scene, ViewportPanelWidget* viewport)
{
    //в логике инструмента "Отрезок" отпускание кнопки не играет роли
    Q_UNUSED(event);
    Q_UNUSED(scene);
    Q_UNUSED(viewport);
}

void SegmentCreationTool::reset()
{
    //состояние сбрасывается в "Покой"
    m_currentState = State::Idle;
    SnapManager::instance().clearBasePoint();
}

void SegmentCreationTool::onPaint(QPainter& painter)
{
    //если запущен процесс создания отрезка
    if (m_currentState == State::WaitingForSecondPoint) {

        //формируем полупрозрачный цвет для предпросмотра
        QColor previewColor = m_currentColor;
        previewColor.setAlpha(150);

        QPointF firstPt(m_firstPoint.getX(), m_firstPoint.getY());
        QPointF currentPt(m_currentMousePos.getX(), m_currentMousePos.getY());

        //используем менеджер стилей для отрисовки того типа линии, который выбран
        LineStyleManager::instance().drawLine(
            painter,
            firstPt,
            currentPt,
            static_cast<int>(m_currentLineType),
            previewColor
            );
        
        // ЖИРНЫЙ МАРКЕР ПЕРВОЙ ТОЧКИ
        painter.setPen(QPen(Qt::white, 2.0));
        painter.setBrush(m_currentColor);
        painter.drawEllipse(firstPt, 6, 6);
    }
}

void SegmentCreationTool::setColor(const QColor& color) { m_currentColor = color; }
void SegmentCreationTool::setLineType(LineType type) { m_currentLineType = type; }

QColor SegmentCreationTool::getColor() const { return m_currentColor; }
