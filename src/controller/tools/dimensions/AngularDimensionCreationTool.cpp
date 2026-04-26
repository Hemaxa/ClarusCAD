#include "AngularDimensionCreationTool.h"

#include "../../../model/Scene.h"
#include "../../../view/managers/ObjectBindingManager.h"
#include "../../../view/managers/SettingsManager.h"
#include "../../../view/managers/SnapManager.h"
#include "../../../view/panels/ViewportPanelWidget.h"
#include "../../../model/primitives/SegmentPrimitive.h"
#include "../../../model/primitives/RectanglePrimitive.h"
#include "../../../model/primitives/PolygonPrimitive.h"

#include <QMouseEvent>
#include <cmath>

namespace {
struct EdgeCandidate {
    bool valid = false;
    BasePrimitive* source = nullptr;
    int edgeIndex = -1;
    QPointF a;
    QPointF b;
    QPointF picked;
    double distance = 0.0;
};

QPointF closestPointOnSegment(const QPointF& p, const QPointF& a, const QPointF& b)
{
    const double dx = b.x() - a.x();
    const double dy = b.y() - a.y();
    const double lenSq = dx * dx + dy * dy;
    if (lenSq < 1e-12) return a;
    const double t = std::clamp(((p.x() - a.x()) * dx + (p.y() - a.y()) * dy) / lenSq, 0.0, 1.0);
    return QPointF(a.x() + t * dx, a.y() + t * dy);
}

bool lineIntersection(const QPointF& a1, const QPointF& a2, const QPointF& b1, const QPointF& b2, QPointF& out)
{
    const double x1 = a1.x(), y1 = a1.y(), x2 = a2.x(), y2 = a2.y();
    const double x3 = b1.x(), y3 = b1.y(), x4 = b2.x(), y4 = b2.y();
    const double denom = (x1 - x2) * (y3 - y4) - (y1 - y2) * (x3 - x4);
    if (std::abs(denom) < 1e-9) return false;
    out = QPointF(
        ((x1 * y2 - y1 * x2) * (x3 - x4) - (x1 - x2) * (x3 * y4 - y3 * x4)) / denom,
        ((x1 * y2 - y1 * x2) * (y3 - y4) - (y1 - y2) * (x3 * y4 - y3 * x4)) / denom
    );
    return true;
}

double normalizeAngle(double angle)
{
    while (angle < 0.0) angle += 2.0 * M_PI;
    while (angle >= 2.0 * M_PI) angle -= 2.0 * M_PI;
    return angle;
}

double positiveSweepAngle(double start, double end)
{
    double delta = normalizeAngle(end) - normalizeAngle(start);
    while (delta < 0.0) delta += 2.0 * M_PI;
    while (delta >= 2.0 * M_PI) delta -= 2.0 * M_PI;
    return delta;
}

bool angleInsideSweep(double testAngle, double startAngle, double endAngle)
{
    return positiveSweepAngle(startAngle, testAngle) <= positiveSweepAngle(startAngle, endAngle);
}

void considerEdge(const QPointF& mousePos, BasePrimitive* source, int edgeIndex, const QPointF& a, const QPointF& b, EdgeCandidate& best)
{
    const QPointF picked = closestPointOnSegment(mousePos, a, b);
    const double dist = QLineF(mousePos, picked).length();
    if (!best.valid || dist < best.distance) {
        best.valid = true;
        best.source = source;
        best.edgeIndex = edgeIndex;
        best.a = a;
        best.b = b;
        best.picked = picked;
        best.distance = dist;
    }
}

EdgeCandidate findEdgeCandidate(Scene* scene, const QPointF& pos, double tolerance)
{
    EdgeCandidate best;
    best.distance = tolerance;
    if (!scene) return best;

    for (const auto& prim : scene->getPrimitives()) {
        switch (prim->getType()) {
        case PrimitiveType::Segment: {
            auto* seg = static_cast<SegmentPrimitive*>(prim.get());
            considerEdge(pos, prim.get(), 0,
                         QPointF(seg->getStart().getX(), seg->getStart().getY()),
                         QPointF(seg->getEnd().getX(), seg->getEnd().getY()),
                         best);
            break;
        }
        case PrimitiveType::Rectangle: {
            QVector<QPointF> pts = prim->getSnapPoints();
            if (pts.size() >= 5) {
                for (int i = 1; i <= 4; ++i) {
                    considerEdge(pos, prim.get(), i - 1, pts[i], pts[(i % 4) + 1], best);
                }
            }
            break;
        }
        case PrimitiveType::Polygon: {
            auto* poly = static_cast<PolygonPrimitive*>(prim.get());
            QVector<QPointF> verts = poly->getVertices();
            for (int i = 0; i < verts.size(); ++i) {
                considerEdge(pos, prim.get(), i, verts[i], verts[(i + 1) % verts.size()], best);
            }
            break;
        }
        default:
            break;
        }
    }

    return best;
}
}

AngularDimensionCreationTool::AngularDimensionCreationTool(QObject* parent)
    : BaseCreationTool(parent)
{
}

void AngularDimensionCreationTool::onMousePress(QMouseEvent* event, Scene* scene, ViewportPanelWidget* viewport)
{
    if (event->button() == Qt::LeftButton) {
        const QPointF pos = viewport->getSnappedPoint(event->position());
        if (m_state == 0) {
            const EdgeCandidate edge = findEdgeCandidate(scene, pos, 18.0 / viewport->getZoomFactor());
            if (edge.valid) {
                m_hasFirstEdge = true;
                m_firstEdgeSource = edge.source;
                m_firstEdgeIndex = edge.edgeIndex;
                m_firstEdgeA = edge.a;
                m_firstEdgeB = edge.b;
                m_firstEdgePicked = edge.picked;
                m_state = 10;
                viewport->update();
                return;
            }

            m_previewDimension = std::make_unique<AngularDimensionPrimitive>();
            m_previewDimension->setCenterPoint(pos);
            m_previewDimension->setStartPoint(pos);
            m_previewDimension->setEndPoint(pos);
            m_previewDimension->setArcPoint(pos);

            DimensionStyle style = SettingsManager::instance().getDefaultDimensionStyle();
            style.dimensionLineColor = m_currentColor;
            style.extensionLineColor = m_currentColor;
            style.textColor = m_currentColor;
            m_previewDimension->setStyle(style);

            SnapManager::instance().setBasePoint(pos);
            m_state = 1;
        } else if (m_state == 10) {
            const EdgeCandidate edge = findEdgeCandidate(scene, pos, 18.0 / viewport->getZoomFactor());
            QPointF center;
            if (edge.valid && lineIntersection(m_firstEdgeA, m_firstEdgeB, edge.a, edge.b, center)) {
                m_previewDimension = std::make_unique<AngularDimensionPrimitive>();
                m_previewDimension->setCenterPoint(center);
                QPointF firstRayPoint = closestPointOnSegment(m_firstEdgePicked, m_firstEdgeA, m_firstEdgeB);
                QPointF secondRayPoint = closestPointOnSegment(edge.picked, edge.a, edge.b);
                const double firstAngle = std::atan2(firstRayPoint.y() - center.y(), firstRayPoint.x() - center.x());
                const double secondAngle = std::atan2(secondRayPoint.y() - center.y(), secondRayPoint.x() - center.x());
                const double pickedAngle = std::atan2(pos.y() - center.y(), pos.x() - center.x());

                if (angleInsideSweep(pickedAngle, firstAngle, secondAngle)) {
                    m_previewDimension->setStartPoint(firstRayPoint);
                    m_previewDimension->setEndPoint(secondRayPoint);
                } else {
                    m_previewDimension->setStartPoint(secondRayPoint);
                    m_previewDimension->setEndPoint(firstRayPoint);
                }

                QPointF avgDir = (m_previewDimension->getStartPoint() - center) + (m_previewDimension->getEndPoint() - center);
                if (QLineF(QPointF(0, 0), avgDir).length() < 1e-6) {
                    avgDir = QPointF(1.0, 0.0);
                }
                const double len = std::max(QLineF(center, m_previewDimension->getStartPoint()).length(),
                                            QLineF(center, m_previewDimension->getEndPoint()).length());
                QLineF dirLine(QPointF(0, 0), avgDir);
                dirLine.setLength(std::max(20.0, len * 0.6));
                m_previewDimension->setArcPoint(center + (dirLine.p2() - QPointF(0, 0)));

                DimensionStyle style = SettingsManager::instance().getDefaultDimensionStyle();
                style.dimensionLineColor = m_currentColor;
                style.extensionLineColor = m_currentColor;
                style.textColor = m_currentColor;
                m_previewDimension->setStyle(style);
                m_previewDimension->setEdgeAssociation(m_firstEdgeSource, m_firstEdgeIndex, edge.source, edge.edgeIndex);
                m_previewDimension->recalculateValue();

                m_hasFirstEdge = false;
                ObjectBindingManager::instance().setPrimitiveSnap(false);
                m_state = 3;
            } else {
                m_hasFirstEdge = false;
                m_firstEdgeSource = nullptr;
                m_firstEdgeIndex = -1;
                m_firstEdgePicked = QPointF();
                m_state = 0;
            }
        } else if (m_state == 1) {
            m_previewDimension->setStartPoint(pos);
            m_state = 2;
        } else if (m_state == 2) {
            m_previewDimension->setEndPoint(pos);
            m_previewDimension->recalculateValue();
            SnapManager::instance().clearBasePoint();
            ObjectBindingManager::instance().setPrimitiveSnap(false);
            m_state = 3;
        } else if (m_state == 3) {
            m_previewDimension->setArcPoint(pos);
            m_previewDimension->recalculateValue();
            emit dimensionDataReady(m_previewDimension.release());
            ObjectBindingManager::instance().setPrimitiveSnap(true);
            m_state = 0;
        }
        viewport->update();
    } else if (event->button() == Qt::RightButton) {
        reset();
        viewport->update();
    }
}

void AngularDimensionCreationTool::onMouseMove(QMouseEvent* event, Scene* scene, ViewportPanelWidget* viewport)
{
    Q_UNUSED(scene);
    if (!m_previewDimension) return;

    const QPointF pos = viewport->getSnappedPoint(event->position());
    if (m_state == 1) {
        m_previewDimension->setStartPoint(pos);
    } else if (m_state == 2) {
        m_previewDimension->setEndPoint(pos);
        m_previewDimension->setArcPoint(pos);
        m_previewDimension->recalculateValue();
    } else if (m_state == 3) {
        m_previewDimension->setArcPoint(pos);
        m_previewDimension->recalculateValue();
    }
    viewport->update();
}

void AngularDimensionCreationTool::onMouseRelease(QMouseEvent* event, Scene* scene, ViewportPanelWidget* viewport)
{
    Q_UNUSED(event);
    Q_UNUSED(scene);
    Q_UNUSED(viewport);
}

void AngularDimensionCreationTool::reset()
{
    m_state = 0;
    m_hasFirstEdge = false;
    m_firstEdgeSource = nullptr;
    m_firstEdgeIndex = -1;
    m_firstEdgePicked = QPointF();
    m_previewDimension.reset();
    SnapManager::instance().clearBasePoint();
    ObjectBindingManager::instance().setPrimitiveSnap(true);
}

void AngularDimensionCreationTool::onPaint(QPainter& painter)
{
    if (m_previewDimension && (m_state >= 1 && m_state <= 3)) {
        m_previewDimension->draw(painter, false);
    }
}

void AngularDimensionCreationTool::setColor(const QColor& color)
{
    m_currentColor = color;
    if (m_previewDimension) {
        m_previewDimension->setColor(color);
    }
}

QColor AngularDimensionCreationTool::getColor() const
{
    return m_currentColor;
}
