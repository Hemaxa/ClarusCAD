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
void drawDimensionArrow(QPainter& painter, const QPointF& tip, double angle, const DimensionStyle& style,
                        const QColor& color, double size)
{
    painter.save();
    painter.setPen(QPen(color, 0));
    painter.setBrush(style.arrowFilled ? color : Qt::NoBrush);

    if (style.arrowType == DimensionArrowType::Slash) {
        const double slashAngle = angle + M_PI / 2.0;
        painter.drawLine(
            QPointF(tip.x() - size * std::cos(slashAngle), tip.y() - size * std::sin(slashAngle)),
            QPointF(tip.x() + size * std::cos(slashAngle), tip.y() + size * std::sin(slashAngle))
        );
    } else if (style.arrowType == DimensionArrowType::Dot) {
        painter.drawEllipse(tip, size * 0.35, size * 0.35);
    } else {
        QPolygonF arrow;
        arrow << tip
              << QPointF(tip.x() + size * std::cos(angle + 0.35), tip.y() + size * std::sin(angle + 0.35))
              << QPointF(tip.x() + size * std::cos(angle - 0.35), tip.y() + size * std::sin(angle - 0.35));
        if (style.arrowType == DimensionArrowType::ClosedOpen) {
            painter.setBrush(Qt::NoBrush);
        }
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
        double angle = a.param;
        if (a.snapType == SnapType::Endpoint) angle = (a.index == 0 ? arc->getStartAngle() : arc->getStartAngle() + arc->getSpanAngle()) * M_PI / 180.0;
        if (a.snapType == SnapType::Midpoint) angle = (arc->getStartAngle() + arc->getSpanAngle() / 2.0) * M_PI / 180.0;
        return QPointF(center.x() + arc->getRadius() * std::cos(angle), center.y() + arc->getRadius() * std::sin(angle));
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
        if (a.snapType == SnapType::Endpoint && a.index >= 0 && a.index < verts.size()) return verts[a.index];
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

    m_endAttachment = {};
    m_endAttachment.fallback = m_endPoint;
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
    
    // Draw dimension line
    QPointF dimDrawStart(dimStart.x() - std::cos(geom.angle) * (m_style.dimensionLineExtension / currentScale),
                         dimStart.y() - std::sin(geom.angle) * (m_style.dimensionLineExtension / currentScale));
    QPointF dimDrawEnd(dimEnd.x() + std::cos(geom.angle) * (m_style.dimensionLineExtension / currentScale),
                       dimEnd.y() + std::sin(geom.angle) * (m_style.dimensionLineExtension / currentScale));
    LineStyleManager::instance().drawLine(painter, dimDrawStart, dimDrawEnd, m_style.dimensionLineTypeId, dimColor, false);
    
    // Draw arrows
    drawDimensionArrow(painter, dimStart, geom.angle, m_style, dimColor, arrowSize);
    drawDimensionArrow(painter, dimEnd, geom.angle + M_PI, m_style, dimColor, arrowSize);
    
    // Draw text
    QString text = m_customText.isEmpty() ? QString::number(m_measuredValue, 'f', 2) : m_customText;
    
    QPointF midPoint = getTextAnchor();
    if (!hasCustomTextPosition()) {
        midPoint += QPointF(std::cos(geom.angle), std::sin(geom.angle)) * (m_style.textAlongLineOffset / currentScale);
    }
    
    const QPointF screenStart = worldTransform.map(dimStart);
    const QPointF screenEnd = worldTransform.map(dimEnd);
    double textAngle = std::atan2(screenEnd.y() - screenStart.y(), screenEnd.x() - screenStart.x()) * 180.0 / M_PI;
    normalizeReadableScreenAngle(textAngle);

    painter.restore();
    painter.save();
    painter.resetTransform();

    QFont font(m_style.fontFamily);
    font.setPixelSize(std::max(1, static_cast<int>(std::round(m_style.textHeight))));
    painter.setFont(font);
    painter.setPen(isSelected ? Qt::red : m_style.textColor);
    painter.translate(worldTransform.map(midPoint));
    painter.rotate(textAngle);
    
    QFontMetricsF fm(font);
    double textWidth = fm.horizontalAdvance(text);
    double textH = fm.height();
    double textOffset = m_style.textGap;
    
    painter.drawText(QRectF(-textWidth / 2.0, -textH - textOffset, textWidth, textH), Qt::AlignCenter, text);
    
    painter.restore();
}

QPointF LinearDimensionPrimitive::getDefaultTextAnchor() const
{
    const LinearGeometry geom = buildGeometry(m_startPoint, m_endPoint, m_dimensionLinePos, m_mode);
    return QPointF((geom.dimStart.x() + geom.dimEnd.x()) / 2.0,
                   (geom.dimStart.y() + geom.dimEnd.y()) / 2.0);
}

QVector<QPointF> LinearDimensionPrimitive::getEditGripPoints() const
{
    return {m_startPoint, m_endPoint, m_dimensionLinePos};
}

void LinearDimensionPrimitive::moveGripPoint(int index, const QPointF& newPos)
{
    if (index == 0) {
        m_startPoint = newPos;
        m_startAttachment = {};
        m_startAttachment.fallback = newPos;
    } else if (index == 1) {
        m_endPoint = newPos;
        m_endAttachment = {};
        m_endAttachment.fallback = newPos;
    } else if (index == 2) {
        m_dimensionLinePos = newPos;
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
    double minX = std::min({m_startPoint.x(), m_endPoint.x(), m_dimensionLinePos.x()});
    double maxX = std::max({m_startPoint.x(), m_endPoint.x(), m_dimensionLinePos.x()});
    double minY = std::min({m_startPoint.y(), m_endPoint.y(), m_dimensionLinePos.y()});
    double maxY = std::max({m_startPoint.y(), m_endPoint.y(), m_dimensionLinePos.y()});

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
