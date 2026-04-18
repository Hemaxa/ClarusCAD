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

void LinearDimensionPrimitive::draw(QPainter& painter, bool isSelected) const {
    painter.save();

    double currentScale = std::abs(painter.transform().m11());
    if (currentScale < 0.0001) currentScale = 1.0;
    
    double textHeight = m_style.textHeight / currentScale;
    double arrowSize = m_style.arrowSize / currentScale;
    double extensionLineOffset = m_style.extensionLineOffset / currentScale;
    double extensionLineExtend = m_style.extensionLineExtend / currentScale;
    
    double dx = m_endPoint.x() - m_startPoint.x();
    double dy = m_endPoint.y() - m_startPoint.y();
    double angle = 0.0;
    double ux = 1.0;
    double uy = 0.0;
    double length = 0.0;

    switch (m_mode) {
    case LinearDimensionMode::Horizontal:
        angle = 0.0;
        ux = 1.0;
        uy = 0.0;
        length = std::abs(dx);
        break;
    case LinearDimensionMode::Vertical:
        angle = M_PI / 2.0;
        ux = 0.0;
        uy = 1.0;
        length = std::abs(dy);
        break;
    case LinearDimensionMode::Aligned:
    default:
        length = std::sqrt(dx * dx + dy * dy);
        if (length >= 0.0001) {
            ux = dx / length;
            uy = dy / length;
            angle = std::atan2(dy, dx);
        }
        break;
    }

    if (length < 0.0001) {
        painter.restore();
        return;
    }
    
    // Normal vector
    double nx = -uy;
    double ny = ux;
    
    // Calculate dimension line distance from startPoint along the normal
    double vx = m_dimensionLinePos.x() - m_startPoint.x();
    double vy = m_dimensionLinePos.y() - m_startPoint.y();
    double dist = vx * nx + vy * ny; 
    
    // Start and End points of the dimension line
    QPointF baseStart = m_startPoint;
    QPointF baseEnd = m_endPoint;
    if (m_mode == LinearDimensionMode::Horizontal) {
        baseStart.setY(m_startPoint.y());
        baseEnd.setY(m_startPoint.y());
        baseEnd.setX(m_endPoint.x());
    } else if (m_mode == LinearDimensionMode::Vertical) {
        baseStart.setX(m_startPoint.x());
        baseEnd.setX(m_startPoint.x());
        baseEnd.setY(m_endPoint.y());
    }

    QPointF dimStart(baseStart.x() + nx * dist, baseStart.y() + ny * dist);
    QPointF dimEnd(baseEnd.x() + nx * dist, baseEnd.y() + ny * dist);
    
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
    QPointF dimDrawStart(dimStart.x() - std::cos(angle) * (m_style.dimensionLineExtension / currentScale),
                         dimStart.y() - std::sin(angle) * (m_style.dimensionLineExtension / currentScale));
    QPointF dimDrawEnd(dimEnd.x() + std::cos(angle) * (m_style.dimensionLineExtension / currentScale),
                       dimEnd.y() + std::sin(angle) * (m_style.dimensionLineExtension / currentScale));
    LineStyleManager::instance().drawLine(painter, dimDrawStart, dimDrawEnd, m_style.dimensionLineTypeId, dimColor, false);
    
    // Draw arrows
    drawDimensionArrow(painter, dimStart, angle, m_style, dimColor, arrowSize);
    drawDimensionArrow(painter, dimEnd, angle + M_PI, m_style, dimColor, arrowSize);
    
    // Draw text
    QString text = m_customText.isEmpty() ? QString::number(m_measuredValue, 'f', 2) : m_customText;
    
    QFont font(m_style.fontFamily);
    font.setPointSizeF(textHeight);
    painter.setFont(font);
    
    QPointF midPoint = getTextAnchor();
    if (!hasCustomTextPosition()) {
        midPoint += QPointF(std::cos(angle), std::sin(angle)) * (m_style.textAlongLineOffset / currentScale);
    }
    
    double textAngle = angle * 180.0 / M_PI;
    
    // Keep text upright
    if (textAngle > 90.0) textAngle -= 180.0;
    else if (textAngle < -90.0) textAngle += 180.0;
    
    painter.translate(midPoint);
    painter.rotate(textAngle);
    painter.setPen(isSelected ? Qt::red : m_style.textColor);
    
    QFontMetricsF fm(font);
    double textWidth = fm.horizontalAdvance(text);
    double textH = fm.height();
    double textOffset = m_style.textGap / currentScale;
    
    painter.drawText(QRectF(-textWidth / 2.0, -textH - textOffset, textWidth, textH), Qt::AlignCenter, text);
    
    painter.restore();
}

QPointF LinearDimensionPrimitive::getDefaultTextAnchor() const
{
    double dx = m_endPoint.x() - m_startPoint.x();
    double dy = m_endPoint.y() - m_startPoint.y();
    double angle = 0.0;

    switch (m_mode) {
    case LinearDimensionMode::Horizontal:
        angle = 0.0;
        break;
    case LinearDimensionMode::Vertical:
        angle = M_PI / 2.0;
        break;
    case LinearDimensionMode::Aligned:
    default:
        angle = std::atan2(dy, dx);
        break;
    }

    QPointF dimStart = m_startPoint;
    QPointF dimEnd = m_endPoint;
    if (m_mode == LinearDimensionMode::Horizontal) {
        dimEnd.setY(dimStart.y());
    } else if (m_mode == LinearDimensionMode::Vertical) {
        dimEnd.setX(dimStart.x());
    }

    QPointF midPoint((dimStart.x() + dimEnd.x()) / 2.0, (dimStart.y() + dimEnd.y()) / 2.0);
    return midPoint + QPointF(std::cos(angle), std::sin(angle)) * m_style.textAlongLineOffset;
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
