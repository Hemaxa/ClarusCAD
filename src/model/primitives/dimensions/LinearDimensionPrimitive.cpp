// LinearDimensionPrimitive.cpp
#include "LinearDimensionPrimitive.h"
#include "../../../view/managers/LineStyleManager.h"
#include "../../primitives/SegmentPrimitive.h"
#include "../../primitives/CirclePrimitive.h"
#include "../../primitives/ArcPrimitive.h"
#include "../../primitives/RectanglePrimitive.h"
#include "../../primitives/EllipsePrimitive.h"
#include "../../primitives/PolygonPrimitive.h"
#include <cmath>
#include <algorithm>

namespace {
QPointF pointOnArcQt(const QPointF& center, double radius, double angleDeg)
{
    const double angleRad = angleDeg * M_PI / 180.0;
    return QPointF(center.x() + radius * std::cos(angleRad),
                   center.y() - radius * std::sin(angleRad));
}

void drawDimensionArrow(QPainter& painter, const QPointF& tip, double angle, const DimensionStyle& style,
                        const QColor& color, double size)
{
    painter.save();
    painter.setPen(QPen(color, 0));
    painter.setBrush(Qt::NoBrush);

    if (style.arrowType == DimensionArrowType::Slash) {
        const double slashAngle = angle + M_PI / 3.0;
        const QPointF slashDir(std::cos(slashAngle), std::sin(slashAngle));
        painter.drawLine(
            tip - slashDir * (size * 0.8),
            tip + slashDir * (size * 0.8)
        );
    } else if (style.arrowType == DimensionArrowType::Dot) {
        painter.setBrush(color);
        painter.drawEllipse(tip, size * 0.35, size * 0.35);
    } else if (style.arrowType == DimensionArrowType::ClosedOpen) {
        painter.drawLine(tip,
                         QPointF(tip.x() + size * std::cos(angle + 0.35), tip.y() + size * std::sin(angle + 0.35)));
        painter.drawLine(tip,
                         QPointF(tip.x() + size * std::cos(angle - 0.35), tip.y() + size * std::sin(angle - 0.35)));
    } else {
        painter.setBrush(style.arrowFilled ? color : Qt::NoBrush);
        QPolygonF arrow;
        arrow << tip
              << QPointF(tip.x() + size * std::cos(angle + 0.35), tip.y() + size * std::sin(angle + 0.35))
              << QPointF(tip.x() + size * std::cos(angle - 0.35), tip.y() + size * std::sin(angle - 0.35));
        painter.drawPolygon(arrow);
    }

    painter.restore();
}

QPointF closestPointOnSegment(const QPointF& p, const QPointF& a, const QPointF& b)
{
    const double dx = b.x() - a.x();
    const double dy = b.y() - a.y();
    const double lenSq = dx * dx + dy * dy;
    if (lenSq < 1e-12) return a;
    const double t = std::clamp(((p.x() - a.x()) * dx + (p.y() - a.y()) * dy) / lenSq, 0.0, 1.0);
    return QPointF(a.x() + t * dx, a.y() + t * dy);
}

QPointF projectPointOnInfiniteLine(const QPointF& point, const QPointF& origin, const QPointF& unitDir)
{
    const QPointF delta = point - origin;
    const double along = delta.x() * unitDir.x() + delta.y() * unitDir.y();
    return origin + unitDir * along;
}

QPointF normalizedDirection(const QPointF& from, const QPointF& to)
{
    const double dx = to.x() - from.x();
    const double dy = to.y() - from.y();
    const double len = std::sqrt(dx * dx + dy * dy);
    if (len < 1e-9) return QPointF(1.0, 0.0);
    return QPointF(dx / len, dy / len);
}

QPointF resolveAttachment(const LinearDimensionPrimitive::Attachment& a)
{
    if (!a.source) return a.fallback;

    switch (a.source->getType()) {
    case PrimitiveType::Segment: {
        auto* seg = static_cast<SegmentPrimitive*>(a.source);
        QPointF p1(seg->getStart().getX(), seg->getStart().getY());
        QPointF p2(seg->getEnd().getX(), seg->getEnd().getY());
        if (a.snapType == SnapType::Endpoint) return a.index == 0 ? p1 : p2;
        if (a.snapType == SnapType::Midpoint) return (p1 + p2) / 2.0;
        if (a.snapType == SnapType::Nearest) return p1 + (p2 - p1) * a.param;
        return a.fallback;
    }
    case PrimitiveType::Circle: {
        auto* c = static_cast<CirclePrimitive*>(a.source);
        QPointF center(c->getCenter().getX(), c->getCenter().getY());
        if (a.snapType == SnapType::Center) return center;
        return QPointF(center.x() + c->getRadius() * std::cos(a.param), center.y() + c->getRadius() * std::sin(a.param));
    }
    case PrimitiveType::Arc: {
        auto* arc = static_cast<ArcPrimitive*>(a.source);
        QPointF center(arc->getCenter().getX(), arc->getCenter().getY());
        if (a.snapType == SnapType::Center) return center;
        if (a.snapType == SnapType::Endpoint) {
            return pointOnArcQt(center, arc->getRadius(),
                                a.index == 0 ? arc->getStartAngle() : (arc->getStartAngle() + arc->getSpanAngle()));
        }
        if (a.snapType == SnapType::Midpoint) {
            return pointOnArcQt(center, arc->getRadius(), arc->getStartAngle() + arc->getSpanAngle() / 2.0);
        }
        return QPointF(center.x() + arc->getRadius() * std::cos(a.param),
                       center.y() + arc->getRadius() * std::sin(a.param));
    }
    case PrimitiveType::Ellipse: {
        auto* e = static_cast<EllipsePrimitive*>(a.source);
        QTransform t;
        t.translate(e->getCenter().getX(), e->getCenter().getY());
        t.rotate(e->getRotation());
        if (a.snapType == SnapType::Center) return QPointF(e->getCenter().getX(), e->getCenter().getY());
        return t.map(QPointF(e->getRadiusX() * std::cos(a.param), e->getRadiusY() * std::sin(a.param)));
    }
    case PrimitiveType::Rectangle: {
        auto* r = static_cast<RectanglePrimitive*>(a.source);
        QVector<QPointF> pts = r->getSnapPoints();
        if (a.snapType == SnapType::Center) return pts.isEmpty() ? a.fallback : pts[0];
        if (a.snapType == SnapType::Endpoint && a.index >= 0 && a.index < 4) return pts[1 + a.index];
        if (a.snapType == SnapType::Midpoint && a.index >= 0 && a.index < 4) return pts[5 + a.index];
        if (a.snapType == SnapType::Nearest && pts.size() >= 5) {
            QPointF p1 = pts[1 + a.index];
            QPointF p2 = pts[1 + ((a.index + 1) % 4)];
            return p1 + (p2 - p1) * a.param;
        }
        return a.fallback;
    }
    case PrimitiveType::Polygon: {
        auto* poly = static_cast<PolygonPrimitive*>(a.source);
        QVector<QPointF> verts = poly->getVertices();
        if (verts.isEmpty()) return a.fallback;
        if (a.snapType == SnapType::Center) return QPointF(poly->getCenter().getX(), poly->getCenter().getY());
        if (a.snapType == SnapType::Endpoint && a.index >= 0 && a.index < verts.size()) return verts[a.index];
        if (a.snapType == SnapType::Midpoint && a.index >= 0 && a.index < verts.size()) {
            return (verts[a.index] + verts[(a.index + 1) % verts.size()]) / 2.0;
        }
        if (a.snapType == SnapType::Nearest && a.index >= 0 && a.index < verts.size()) {
            QPointF p1 = verts[a.index];
            QPointF p2 = verts[(a.index + 1) % verts.size()];
            return p1 + (p2 - p1) * a.param;
        }
        return a.fallback;
    }
    default:
        return a.fallback;
    }
}

struct LinearGeometry {
    QPointF baseStart;
    QPointF baseEnd;
    QPointF dimStart;
    QPointF dimEnd;
    double angle = 0.0;
    double length = 0.0;
};

LinearGeometry buildGeometry(const QPointF& startPoint, const QPointF& endPoint,
                             const QPointF& dimensionLinePos, LinearDimensionMode mode)
{
    LinearGeometry g;
    double dx = endPoint.x() - startPoint.x();
    double dy = endPoint.y() - startPoint.y();
    double ux = 1.0;
    double uy = 0.0;

    switch (mode) {
    case LinearDimensionMode::Horizontal:
        g.angle = 0.0;
        ux = 1.0;
        uy = 0.0;
        g.length = std::abs(dx);
        g.baseStart = startPoint;
        g.baseEnd = QPointF(endPoint.x(), startPoint.y());
        break;
    case LinearDimensionMode::Vertical:
        g.angle = M_PI / 2.0;
        ux = 0.0;
        uy = 1.0;
        g.length = std::abs(dy);
        g.baseStart = startPoint;
        g.baseEnd = QPointF(startPoint.x(), endPoint.y());
        break;
    case LinearDimensionMode::Aligned:
    default:
        g.length = std::sqrt(dx * dx + dy * dy);
        if (g.length >= 0.0001) {
            ux = dx / g.length;
            uy = dy / g.length;
            g.angle = std::atan2(dy, dx);
        }
        g.baseStart = startPoint;
        g.baseEnd = endPoint;
        break;
    }

    const double nx = -uy;
    const double ny = ux;
    const double dist = (dimensionLinePos.x() - g.baseStart.x()) * nx
                      + (dimensionLinePos.y() - g.baseStart.y()) * ny;
    g.dimStart = QPointF(g.baseStart.x() + nx * dist, g.baseStart.y() + ny * dist);
    g.dimEnd = QPointF(g.baseEnd.x() + nx * dist, g.baseEnd.y() + ny * dist);
    return g;
}

void normalizeReadableScreenAngle(double& angleDeg)
{
    while (angleDeg <= -180.0) angleDeg += 360.0;
    while (angleDeg > 180.0) angleDeg -= 360.0;
    if (angleDeg > 90.0) angleDeg -= 180.0;
    if (angleDeg < -90.0) angleDeg += 180.0;
}

bool moveAttachmentPoint(LinearDimensionPrimitive::Attachment& attachment, const QPointF& newPoint)
{
    if (!attachment.source) {
        attachment.fallback = newPoint;
        return true;
    }

    switch (attachment.source->getType()) {
    case PrimitiveType::Segment: {
        auto* seg = static_cast<SegmentPrimitive*>(attachment.source);
        if (attachment.snapType != SnapType::Endpoint) return false;
        if (attachment.index == 0) {
            seg->setStart(PointPrimitive::fromPointF(newPoint));
        } else {
            seg->setEnd(PointPrimitive::fromPointF(newPoint));
        }
        attachment.fallback = newPoint;
        return true;
    }
    case PrimitiveType::Rectangle: {
        auto* rect = static_cast<RectanglePrimitive*>(attachment.source);
        if (attachment.snapType != SnapType::Endpoint && attachment.snapType != SnapType::Midpoint && attachment.snapType != SnapType::Nearest) {
            return false;
        }
        attachment.fallback = newPoint;
        Q_UNUSED(rect);
        return true;
    }
    default:
        return false;
    }
}

double rectAttachmentFraction(const LinearDimensionPrimitive::Attachment& attachment)
{
    switch (attachment.snapType) {
    case SnapType::Endpoint:
        return (attachment.index % 2 == 0) ? 0.0 : 1.0;
    case SnapType::Midpoint:
        return 0.5;
    case SnapType::Nearest:
        return attachment.param;
    default:
        return 0.0;
    }
}

bool isRectangleSameEdge(const LinearDimensionPrimitive::Attachment& a, const LinearDimensionPrimitive::Attachment& b, int& edgeIndex)
{
    if (a.snapType == SnapType::Midpoint || a.snapType == SnapType::Nearest) {
        edgeIndex = a.index;
    } else if (a.snapType == SnapType::Endpoint && b.snapType == SnapType::Endpoint) {
        if ((a.index + 1) % 4 == b.index) edgeIndex = a.index;
        else if ((b.index + 1) % 4 == a.index) edgeIndex = b.index;
        else return false;
    } else if (a.snapType == SnapType::Endpoint) {
        if (a.index == b.index) edgeIndex = a.index;
        else if ((a.index + 1) % 4 == b.index) edgeIndex = a.index;
        else if ((b.index + 1) % 4 == a.index) edgeIndex = b.index;
        else return false;
    } else {
        return false;
    }

    if (b.snapType == SnapType::Midpoint || b.snapType == SnapType::Nearest) {
        return b.index == edgeIndex;
    }
    if (b.snapType == SnapType::Endpoint) {
        return b.index == edgeIndex || b.index == ((edgeIndex + 1) % 4);
    }
    return false;
}

bool resizeAssociatedSegment(const LinearDimensionPrimitive::Attachment& startAttachment,
                             const LinearDimensionPrimitive::Attachment& endAttachment,
                             double value)
{
    if (!startAttachment.source || startAttachment.source != endAttachment.source) return false;
    if (startAttachment.source->getType() != PrimitiveType::Segment) return false;

    auto* seg = static_cast<SegmentPrimitive*>(startAttachment.source);
    QPointF p1(seg->getStart().getX(), seg->getStart().getY());
    QPointF p2(seg->getEnd().getX(), seg->getEnd().getY());
    const QPointF dir = normalizedDirection(p1, p2);

    double t1 = 0.0;
    double t2 = 1.0;
    auto fractionFor = [](const LinearDimensionPrimitive::Attachment& attachment) {
        if (attachment.snapType == SnapType::Endpoint) return attachment.index == 0 ? 0.0 : 1.0;
        if (attachment.snapType == SnapType::Midpoint) return 0.5;
        if (attachment.snapType == SnapType::Nearest) return attachment.param;
        return 0.0;
    };
    t1 = fractionFor(startAttachment);
    t2 = fractionFor(endAttachment);
    const double span = std::abs(t2 - t1);
    if (span < 1e-6) return false;

    const double fullLength = value / span;
    seg->setEnd(PointPrimitive::fromPointF(p1 + dir * fullLength));
    return true;
}

bool resizeAssociatedRectangle(const LinearDimensionPrimitive::Attachment& startAttachment,
                               const LinearDimensionPrimitive::Attachment& endAttachment,
                               LinearDimensionMode mode,
                               double value)
{
    if (!startAttachment.source || startAttachment.source != endAttachment.source) return false;
    if (startAttachment.source->getType() != PrimitiveType::Rectangle) return false;

    auto* rect = static_cast<RectanglePrimitive*>(startAttachment.source);
    if (startAttachment.snapType == SnapType::Center || endAttachment.snapType == SnapType::Center) {
        const bool widthDriven = (mode == LinearDimensionMode::Horizontal)
            || (mode == LinearDimensionMode::Aligned && std::abs(resolveAttachment(startAttachment).x() - resolveAttachment(endAttachment).x())
                >= std::abs(resolveAttachment(startAttachment).y() - resolveAttachment(endAttachment).y()));
        if (widthDriven) {
            rect->setWidth(value * 2.0);
        } else {
            rect->setHeight(value * 2.0);
        }
        return true;
    }

    int edgeIndex = -1;
    if (isRectangleSameEdge(startAttachment, endAttachment, edgeIndex)) {
        const double t1 = rectAttachmentFraction(startAttachment);
        const double t2 = rectAttachmentFraction(endAttachment);
        const double span = std::abs(t2 - t1);
        if (span >= 1e-6) {
            const double fullEdgeLength = value / span;
            if (edgeIndex % 2 == 0) rect->setWidth(fullEdgeLength);
            else rect->setHeight(fullEdgeLength);
            return true;
        }
    }

    if (mode == LinearDimensionMode::Horizontal) {
        rect->setWidth(value);
    } else if (mode == LinearDimensionMode::Vertical) {
        rect->setHeight(value);
    } else {
        const QPointF p1 = resolveAttachment(startAttachment);
        const QPointF p2 = resolveAttachment(endAttachment);
        const double dx = std::abs(p2.x() - p1.x());
        const double dy = std::abs(p2.y() - p1.y());
        if (dx >= dy) {
            rect->setWidth(value);
        } else {
            rect->setHeight(value);
        }
    }
    return true;
}

bool resizeAssociatedPolygon(const LinearDimensionPrimitive::Attachment& startAttachment,
                             const LinearDimensionPrimitive::Attachment& endAttachment,
                             double value)
{
    if (!startAttachment.source || startAttachment.source != endAttachment.source) return false;
    if (startAttachment.source->getType() != PrimitiveType::Polygon) return false;

    auto* poly = static_cast<PolygonPrimitive*>(startAttachment.source);
    const int sides = poly->getSides();
    if (sides < 3) return false;

    auto sideLengthToStoredRadius = [poly, sides](double sideLength) {
        if (poly->getPolygonType() == PolygonType::Inscribed) {
            return sideLength / (2.0 * std::sin(M_PI / sides));
        }
        return sideLength / (2.0 * std::tan(M_PI / sides));
    };

    if (startAttachment.snapType == SnapType::Center || endAttachment.snapType == SnapType::Center) {
        const LinearDimensionPrimitive::Attachment& edgeAttachment =
            (startAttachment.snapType == SnapType::Center) ? endAttachment : startAttachment;
        if (edgeAttachment.snapType == SnapType::Endpoint) {
            const double storedRadius = poly->getPolygonType() == PolygonType::Inscribed
                ? value
                : value * std::cos(M_PI / sides);
            poly->setRadius(storedRadius);
            return true;
        }
        if (edgeAttachment.snapType == SnapType::Midpoint || edgeAttachment.snapType == SnapType::Nearest) {
            const double storedRadius = poly->getPolygonType() == PolygonType::Inscribed
                ? value / std::cos(M_PI / sides)
                : value;
            poly->setRadius(storedRadius);
            return true;
        }
    }

    auto edgeOf = [sides](const LinearDimensionPrimitive::Attachment& attachment) {
        if (attachment.snapType == SnapType::Midpoint || attachment.snapType == SnapType::Nearest) return attachment.index;
        return attachment.index % sides;
    };
    auto fractionOf = [sides](const LinearDimensionPrimitive::Attachment& attachment) {
        if (attachment.snapType == SnapType::Midpoint) return 0.5;
        if (attachment.snapType == SnapType::Nearest) return attachment.param;
        return 0.0;
    };

    int edgeIndex = -1;
    if (startAttachment.snapType == SnapType::Endpoint && endAttachment.snapType == SnapType::Endpoint) {
        if ((startAttachment.index + 1) % sides == endAttachment.index) edgeIndex = startAttachment.index;
        else if ((endAttachment.index + 1) % sides == startAttachment.index) edgeIndex = endAttachment.index;
    } else {
        edgeIndex = edgeOf(startAttachment);
        if (edgeOf(endAttachment) != edgeIndex) edgeIndex = -1;
    }

    if (edgeIndex >= 0) {
        double t1 = 0.0;
        double t2 = 1.0;
        if (startAttachment.snapType == SnapType::Endpoint) t1 = (startAttachment.index == edgeIndex) ? 0.0 : 1.0;
        else t1 = fractionOf(startAttachment);
        if (endAttachment.snapType == SnapType::Endpoint) t2 = (endAttachment.index == edgeIndex) ? 0.0 : 1.0;
        else t2 = fractionOf(endAttachment);
        const double span = std::abs(t2 - t1);
        if (span >= 1e-6) {
            poly->setRadius(sideLengthToStoredRadius(value / span));
            return true;
        }
    }

    return false;
}
}

void LinearDimensionPrimitive::recalculateValue() {
    double dx = m_endPoint.x() - m_startPoint.x();
    double dy = m_endPoint.y() - m_startPoint.y();
    switch (m_mode) {
    case LinearDimensionMode::Horizontal:
        m_measuredValue = std::abs(dx);
        break;
    case LinearDimensionMode::Vertical:
        m_measuredValue = std::abs(dy);
        break;
    case LinearDimensionMode::Aligned:
    default:
        m_measuredValue = std::sqrt(dx * dx + dy * dy);
        break;
    }
}

bool LinearDimensionPrimitive::applyMeasuredValueOverride(double value)
{
    if (value <= 0.0) return false;

    if (resizeAssociatedSegment(m_startAttachment, m_endAttachment, value)
        || resizeAssociatedRectangle(m_startAttachment, m_endAttachment, m_mode, value)
        || resizeAssociatedPolygon(m_startAttachment, m_endAttachment, value)) {
        updateFromAttachments();
        return true;
    }

    switch (m_mode) {
    case LinearDimensionMode::Horizontal: {
        const double sign = (m_endPoint.x() >= m_startPoint.x()) ? 1.0 : -1.0;
        m_endPoint.setX(m_startPoint.x() + sign * value);
        break;
    }
    case LinearDimensionMode::Vertical: {
        const double sign = (m_endPoint.y() >= m_startPoint.y()) ? 1.0 : -1.0;
        m_endPoint.setY(m_startPoint.y() + sign * value);
        break;
    }
    case LinearDimensionMode::Aligned:
    default: {
        const double dx = m_endPoint.x() - m_startPoint.x();
        const double dy = m_endPoint.y() - m_startPoint.y();
        const double len = std::sqrt(dx * dx + dy * dy);
        if (len < 1e-6) return false;
        m_endPoint = QPointF(m_startPoint.x() + dx / len * value,
                             m_startPoint.y() + dy / len * value);
        break;
    }
    }

    if (!moveAttachmentPoint(m_endAttachment, m_endPoint)) {
        m_endAttachment = {};
        m_endAttachment.fallback = m_endPoint;
    } else {
        updateFromAttachments();
        return true;
    }

    recalculateValue();
    return true;
}

void LinearDimensionPrimitive::draw(QPainter& painter, bool isSelected) const {
    painter.save();

    const QTransform worldTransform = painter.transform();
    double currentScale = std::abs(worldTransform.m11());
    if (currentScale < 0.0001) currentScale = 1.0;
    
    double arrowSize = m_style.arrowSize / currentScale;
    double extensionLineOffset = m_style.extensionLineOffset / currentScale;
    double extensionLineExtend = m_style.extensionLineExtend / currentScale;
    
    const LinearGeometry geom = buildGeometry(m_startPoint, m_endPoint, m_dimensionLinePos, m_mode);
    if (geom.length < 0.0001) {
        painter.restore();
        return;
    }
    
    // Normal vector
    double nx = -std::sin(geom.angle);
    double ny = std::cos(geom.angle);
    
    // Calculate dimension line distance from startPoint along the normal
    double vx = m_dimensionLinePos.x() - geom.baseStart.x();
    double vy = m_dimensionLinePos.y() - geom.baseStart.y();
    double dist = vx * nx + vy * ny; 
    
    // Start and End points of the dimension line
    QPointF baseStart = geom.baseStart;
    QPointF baseEnd = geom.baseEnd;
    QPointF dimStart = geom.dimStart;
    QPointF dimEnd = geom.dimEnd;
    
    // Direction of extension lines (from startPoint to dimStart)
    double extLen = std::abs(dist);
    double dirExtX = (extLen > 0) ? (nx * dist / extLen) : nx;
    double dirExtY = (extLen > 0) ? (ny * dist / extLen) : ny;
    
    QPointF extStart1, extStart2;
    if (extLen > extensionLineOffset) {
        extStart1 = QPointF(baseStart.x() + dirExtX * extensionLineOffset, baseStart.y() + dirExtY * extensionLineOffset);
        extStart2 = QPointF(baseEnd.x() + dirExtX * extensionLineOffset, baseEnd.y() + dirExtY * extensionLineOffset);
    } else {
        extStart1 = baseStart;
        extStart2 = baseEnd;
    }
    
    QPointF extEnd1(dimStart.x() + dirExtX * extensionLineExtend, dimStart.y() + dirExtY * extensionLineExtend);
    QPointF extEnd2(dimEnd.x() + dirExtX * extensionLineExtend, dimEnd.y() + dirExtY * extensionLineExtend);
    
    // Draw extension lines
    const QColor extensionColor = isSelected ? Qt::red : m_style.extensionLineColor;
    const QColor dimColor = isSelected ? Qt::red : m_style.dimensionLineColor;
    LineStyleManager::instance().drawLine(painter, extStart1, extEnd1, m_style.extensionLineTypeId, extensionColor, false);
    LineStyleManager::instance().drawLine(painter, extStart2, extEnd2, m_style.extensionLineTypeId, extensionColor, false);
    
    QString text = m_customText.isEmpty()
        ? QString("%1%2").arg(m_textPrefix, QString::number(m_measuredValue, 'f', 2))
        : m_customText;

    QPointF midPoint = getTextAnchor();
    const QPointF lineDir(std::cos(geom.angle), std::sin(geom.angle));
    if (!hasCustomTextPosition()) {
        midPoint += lineDir * (m_style.textAlongLineOffset / currentScale);
    }
    const bool useShelf = hasShelf();

    QFont font(m_style.fontFamily);
    font.setPixelSize(std::max(1, static_cast<int>(std::round(m_style.textHeight))));
    QFontMetricsF fm(font);
    const double textHalfWidthWorld = fm.horizontalAdvance(text) / (2.0 * currentScale);
    const double textMarginWorld = std::max(2.0, m_style.textGap * 0.25) / currentScale;
    const double textAlong = (midPoint.x() - dimStart.x()) * lineDir.x()
                           + (midPoint.y() - dimStart.y()) * lineDir.y();
    const double minAlong = useShelf
        ? std::min(0.0, textAlong)
        : std::min(0.0, textAlong - textHalfWidthWorld - textMarginWorld);
    const double maxAlong = useShelf
        ? std::max(geom.length, textAlong)
        : std::max(geom.length, textAlong + textHalfWidthWorld + textMarginWorld);
    const double dimExtension = m_style.dimensionLineExtension / currentScale;

    // When the label is dragged beyond an extension line, keep the dimension line attached and extend it.
    QPointF dimDrawStart = dimStart + lineDir * (minAlong - dimExtension);
    QPointF dimDrawEnd = dimStart + lineDir * (maxAlong + dimExtension);
    LineStyleManager::instance().drawLine(painter, dimDrawStart, dimDrawEnd, m_style.dimensionLineTypeId, dimColor, false);
    
    // Draw arrows
    drawDimensionArrow(painter, dimStart, geom.angle, m_style, dimColor, arrowSize);
    drawDimensionArrow(painter, dimEnd, geom.angle + M_PI, m_style, dimColor, arrowSize);
    
    const QPointF screenStart = worldTransform.map(dimStart);
    const QPointF screenEnd = worldTransform.map(dimEnd);
    double textAngle = std::atan2(screenEnd.y() - screenStart.y(), screenEnd.x() - screenStart.x()) * 180.0 / M_PI;
    normalizeReadableScreenAngle(textAngle);

    painter.restore();
    painter.save();
    painter.resetTransform();

    painter.setFont(font);
    painter.setPen(isSelected ? Qt::red : m_style.textColor);
    double textWidth = fm.horizontalAdvance(text);
    double textH = fm.height();
    double textOffset = m_style.textGap;

    if (useShelf) {
        const QPointF dimMid((dimStart.x() + dimEnd.x()) * 0.5, (dimStart.y() + dimEnd.y()) * 0.5);
        const QPointF outward = dist >= 0.0 ? QPointF(nx, ny) : QPointF(-nx, -ny);
        const double leaderRise = std::max(m_style.textGap, m_style.textHeight * 0.9) / currentScale;
        double shelfDir = 0.0;
        if (std::abs(outward.x()) > 0.2) {
            shelfDir = outward.x() >= 0.0 ? 1.0 : -1.0;
        } else {
            shelfDir = midPoint.x() >= dimMid.x() ? 1.0 : -1.0;
        }
        const double shelfMargin = std::max(4.0 / currentScale, leaderRise * 0.35);
        const double shelfLength = std::max(textWidth / currentScale + shelfMargin * 2.0, leaderRise * 1.5);
        const QPointF elbow = midPoint + outward * leaderRise;
        const QPointF shelfEnd = elbow + QPointF(shelfDir * shelfLength, 0.0);
        const QPointF screenAnchor = worldTransform.map(midPoint);
        const QPointF screenElbow = worldTransform.map(elbow);
        const QPointF screenShelfEnd = worldTransform.map(shelfEnd);
        painter.setPen(isSelected ? Qt::red : dimColor);
        painter.drawLine(screenAnchor, screenElbow);
        painter.drawLine(screenElbow, screenShelfEnd);
        painter.setPen(isSelected ? Qt::red : m_style.textColor);
        const QPointF shelfCenter = (screenElbow + screenShelfEnd) * 0.5;
        painter.drawText(QRectF(shelfCenter.x() - textWidth / 2.0, shelfCenter.y() - textH - textOffset,
                                textWidth, textH), Qt::AlignCenter, text);
    } else {
        painter.translate(worldTransform.map(midPoint));
        painter.rotate(textAngle);
        painter.drawText(QRectF(-textWidth / 2.0, -textH - textOffset, textWidth, textH), Qt::AlignCenter, text);
    }
    
    painter.restore();
}

QPointF LinearDimensionPrimitive::getDefaultTextAnchor() const
{
    const LinearGeometry geom = buildGeometry(m_startPoint, m_endPoint, m_dimensionLinePos, m_mode);
    return QPointF((geom.dimStart.x() + geom.dimEnd.x()) / 2.0,
                   (geom.dimStart.y() + geom.dimEnd.y()) / 2.0);
}

QPointF LinearDimensionPrimitive::constrainTextAnchor(const QPointF& pos) const
{
    const LinearGeometry geom = buildGeometry(m_startPoint, m_endPoint, m_dimensionLinePos, m_mode);
    if (geom.length < 1e-6) {
        return getDefaultTextAnchor();
    }

    const QPointF lineDir(std::cos(geom.angle), std::sin(geom.angle));
    return projectPointOnInfiniteLine(pos, geom.dimStart, lineDir);
}

QVector<QPointF> LinearDimensionPrimitive::getEditGripPoints() const
{
    return {m_startPoint, m_endPoint, m_dimensionLinePos};
}

void LinearDimensionPrimitive::moveGripPoint(int index, const QPointF& newPos)
{
    const LinearGeometry oldGeom = buildGeometry(m_startPoint, m_endPoint, m_dimensionLinePos, m_mode);
    if (index == 0) {
        m_startPoint = newPos;
        m_startAttachment = {};
        m_startAttachment.fallback = newPos;
    } else if (index == 1) {
        m_endPoint = newPos;
        m_endAttachment = {};
        m_endAttachment.fallback = newPos;
    } else if (index == 2) {
        if (hasCustomTextPosition() && oldGeom.length >= 1e-6) {
            const QPointF lineDir(std::cos(oldGeom.angle), std::sin(oldGeom.angle));
            const double along = QPointF::dotProduct(getCustomTextPosition() - oldGeom.dimStart, lineDir);
            m_dimensionLinePos = newPos;
            const LinearGeometry newGeom = buildGeometry(m_startPoint, m_endPoint, m_dimensionLinePos, m_mode);
            m_customTextPosition = newGeom.dimStart + lineDir * along;
        } else {
            m_dimensionLinePos = newPos;
        }
    }
    recalculateValue();
}

void LinearDimensionPrimitive::updateFromAttachments()
{
    m_startPoint = resolveAttachment(m_startAttachment);
    m_endPoint = resolveAttachment(m_endAttachment);
    recalculateValue();
}

QRectF LinearDimensionPrimitive::getBoundingBox() const {
    // Упрощенный вариант, охватывающий 3 основные точки (start, end, pos)
    QPointF textAnchor = getTextAnchor();
    double minX = std::min({m_startPoint.x(), m_endPoint.x(), m_dimensionLinePos.x(), textAnchor.x()});
    double maxX = std::max({m_startPoint.x(), m_endPoint.x(), m_dimensionLinePos.x(), textAnchor.x()});
    double minY = std::min({m_startPoint.y(), m_endPoint.y(), m_dimensionLinePos.y(), textAnchor.y()});
    double maxY = std::max({m_startPoint.y(), m_endPoint.y(), m_dimensionLinePos.y(), textAnchor.y()});

    if (hasShelf()) {
        const double shelfReach = std::max(60.0, m_style.textHeight * 6.0);
        minX -= shelfReach;
        maxX += shelfReach;
        maxY += shelfReach * 0.5;
        minY -= shelfReach * 0.5;
    }

    QRectF bbox(QPointF(minX, minY), QPointF(maxX, maxY));

    bbox.adjust(-25.0, -25.0, 25.0, 25.0);
    
    return bbox;
}

bool LinearDimensionPrimitive::hitTest(const QPointF& point, double tolerance) const {
    // Упрощенный вариант: проверка попадания в расширенный bounding box
    QRectF bbox = getBoundingBox();
    bbox.adjust(-tolerance, -tolerance, tolerance, tolerance);
    return bbox.contains(point);
}

bool LinearDimensionPrimitive::intersects(const QRectF& rect) const {
    return getBoundingBox().intersects(rect);
}

bool LinearDimensionPrimitive::inside(const QRectF& rect) const {
    return rect.contains(getBoundingBox());
}

QVector<QPointF> LinearDimensionPrimitive::getSnapPoints() const {
    return {m_startPoint, m_endPoint, m_dimensionLinePos};
}
