// AngularDimensionPrimitive.cpp

#include "AngularDimensionPrimitive.h"
#include "../../../view/managers/LineStyleManager.h"
#include "../../primitives/SegmentPrimitive.h"
#include "../../primitives/RectanglePrimitive.h"
#include "../../primitives/PolygonPrimitive.h"

#include <algorithm>
#include <cmath>

namespace {
double pointDistance(const QPointF& a, const QPointF& b)
{
    const double dx = b.x() - a.x();
    const double dy = b.y() - a.y();
    return std::sqrt(dx * dx + dy * dy);
}

double normalizeAngle(double angle)
{
    while (angle < 0.0) angle += 2.0 * M_PI;
    while (angle >= 2.0 * M_PI) angle -= 2.0 * M_PI;
    return angle;
}

double deltaAngle(double start, double end)
{
    double delta = normalizeAngle(end) - normalizeAngle(start);
    while (delta < 0.0) delta += 2.0 * M_PI;
    while (delta >= 2.0 * M_PI) delta -= 2.0 * M_PI;
    if (delta > M_PI) {
        delta -= 2.0 * M_PI;
    }
    return delta;
}

void drawArrow(QPainter& painter, const QPointF& tip, double angle, const DimensionStyle& style,
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

void normalizeReadableScreenAngle(double& angleDeg)
{
    while (angleDeg <= -180.0) angleDeg += 360.0;
    while (angleDeg > 180.0) angleDeg -= 360.0;
    if (angleDeg > 90.0) angleDeg -= 180.0;
    if (angleDeg < -90.0) angleDeg += 180.0;
}

bool lineIntersection(const QPointF& a1, const QPointF& a2, const QPointF& b1, const QPointF& b2, QPointF& out)
{
    const double denom = (a1.x() - a2.x()) * (b1.y() - b2.y()) - (a1.y() - a2.y()) * (b1.x() - b2.x());
    if (std::abs(denom) < 1e-9) return false;
    out = QPointF(
        ((a1.x()*a2.y() - a1.y()*a2.x()) * (b1.x() - b2.x()) - (a1.x() - a2.x()) * (b1.x()*b2.y() - b1.y()*b2.x())) / denom,
        ((a1.x()*a2.y() - a1.y()*a2.x()) * (b1.y() - b2.y()) - (a1.y() - a2.y()) * (b1.x()*b2.y() - b1.y()*b2.x())) / denom
    );
    return true;
}

bool edgePoints(BasePrimitive* source, int edgeIndex, QPointF& a, QPointF& b)
{
    if (!source) return false;
    switch (source->getType()) {
    case PrimitiveType::Segment: {
        auto* seg = static_cast<SegmentPrimitive*>(source);
        a = QPointF(seg->getStart().getX(), seg->getStart().getY());
        b = QPointF(seg->getEnd().getX(), seg->getEnd().getY());
        return true;
    }
    case PrimitiveType::Rectangle: {
        auto* rect = static_cast<RectanglePrimitive*>(source);
        QVector<QPointF> pts = rect->getSnapPoints();
        if (pts.size() < 5) return false;
        a = pts[1 + edgeIndex];
        b = pts[1 + ((edgeIndex + 1) % 4)];
        return true;
    }
    case PrimitiveType::Polygon: {
        auto* poly = static_cast<PolygonPrimitive*>(source);
        QVector<QPointF> verts = poly->getVertices();
        if (verts.isEmpty()) return false;
        a = verts[edgeIndex % verts.size()];
        b = verts[(edgeIndex + 1) % verts.size()];
        return true;
    }
    default:
        return false;
    }
}

QPointF rotateAround(const QPointF& point, const QPointF& center, double angleDelta)
{
    const double dx = point.x() - center.x();
    const double dy = point.y() - center.y();
    const double c = std::cos(angleDelta);
    const double s = std::sin(angleDelta);
    return QPointF(center.x() + dx * c - dy * s,
                   center.y() + dx * s + dy * c);
}
}

void AngularDimensionPrimitive::recalculateValue()
{
    const double startAngle = std::atan2(m_startPoint.y() - m_centerPoint.y(), m_startPoint.x() - m_centerPoint.x());
    const double endAngle = std::atan2(m_endPoint.y() - m_centerPoint.y(), m_endPoint.x() - m_centerPoint.x());
    m_measuredValue = std::abs(deltaAngle(startAngle, endAngle)) * 180.0 / M_PI;
}

bool AngularDimensionPrimitive::applyMeasuredValueOverride(double value)
{
    if (value <= 0.0 || value >= 360.0) return false;

    const double startAngle = std::atan2(m_startPoint.y() - m_centerPoint.y(), m_startPoint.x() - m_centerPoint.x());
    const double endAngle = std::atan2(m_endPoint.y() - m_centerPoint.y(), m_endPoint.x() - m_centerPoint.x());
    const double currentSweep = deltaAngle(startAngle, endAngle);
    const double sign = currentSweep >= 0.0 ? 1.0 : -1.0;
    const double newEndAngle = startAngle + sign * value * M_PI / 180.0;
    const double endRadius = pointDistance(m_centerPoint, m_endPoint);
    if (endRadius < 1e-6) return false;

    if (m_secondSource && m_secondSource->getType() == PrimitiveType::Segment) {
        auto* segment = static_cast<SegmentPrimitive*>(m_secondSource);
        const double currentEndAngle = std::atan2(m_endPoint.y() - m_centerPoint.y(), m_endPoint.x() - m_centerPoint.x());
        const double angleDelta = newEndAngle - currentEndAngle;
        QPointF p1(segment->getStart().getX(), segment->getStart().getY());
        QPointF p2(segment->getEnd().getX(), segment->getEnd().getY());
        p1 = rotateAround(p1, m_centerPoint, angleDelta);
        p2 = rotateAround(p2, m_centerPoint, angleDelta);
        segment->setStart(PointPrimitive::fromPointF(p1));
        segment->setEnd(PointPrimitive::fromPointF(p2));
        updateFromAssociation();
        return true;
    }

    m_endPoint = QPointF(m_centerPoint.x() + endRadius * std::cos(newEndAngle),
                         m_centerPoint.y() + endRadius * std::sin(newEndAngle));
    recalculateValue();
    return true;
}

void AngularDimensionPrimitive::draw(QPainter& painter, bool isSelected) const
{
    painter.save();

    const QTransform worldTransform = painter.transform();
    double currentScale = std::abs(worldTransform.m11());
    if (currentScale < 0.0001) currentScale = 1.0;

    const double radius = pointDistance(m_centerPoint, m_arcPoint);
    if (radius < 1e-6) {
        painter.restore();
        return;
    }

    const double startAngleRad = std::atan2(m_startPoint.y() - m_centerPoint.y(), m_startPoint.x() - m_centerPoint.x());
    const double endAngleRad = std::atan2(m_endPoint.y() - m_centerPoint.y(), m_endPoint.x() - m_centerPoint.x());
    const double sweepRad = deltaAngle(startAngleRad, endAngleRad);
    if (std::abs(sweepRad) < 1e-6) {
        painter.restore();
        return;
    }

    const QPointF startArcPoint(
        m_centerPoint.x() + radius * std::cos(startAngleRad),
        m_centerPoint.y() + radius * std::sin(startAngleRad)
    );
    const QPointF endArcPoint(
        m_centerPoint.x() + radius * std::cos(endAngleRad),
        m_centerPoint.y() + radius * std::sin(endAngleRad)
    );

    const QColor extColor = isSelected ? Qt::red : m_style.extensionLineColor;
    const QColor dimColor = isSelected ? Qt::red : m_style.dimensionLineColor;
    LineStyleManager::instance().drawLine(painter, m_centerPoint, QPointF(
        m_centerPoint.x() + (radius + m_style.extensionLineExtend / currentScale) * std::cos(startAngleRad),
        m_centerPoint.y() + (radius + m_style.extensionLineExtend / currentScale) * std::sin(startAngleRad)),
        m_style.extensionLineTypeId, extColor, false);
    LineStyleManager::instance().drawLine(painter, m_centerPoint, QPointF(
        m_centerPoint.x() + (radius + m_style.extensionLineExtend / currentScale) * std::cos(endAngleRad),
        m_centerPoint.y() + (radius + m_style.extensionLineExtend / currentScale) * std::sin(endAngleRad)),
        m_style.extensionLineTypeId, extColor, false);

    QRectF arcRect(m_centerPoint.x() - radius, m_centerPoint.y() - radius, radius * 2.0, radius * 2.0);
    QPen arcPen = LineStyleManager::instance().getPen(m_style.dimensionLineTypeId, dimColor, false);
    arcPen.setCosmetic(true);
    painter.setPen(arcPen);
    painter.setBrush(Qt::NoBrush);
    painter.drawArc(arcRect, static_cast<int>(-startAngleRad * 180.0 / M_PI * 16.0), static_cast<int>(-sweepRad * 180.0 / M_PI * 16.0));

    const double arrowSize = m_style.arrowSize / currentScale;
    const double directionSign = sweepRad >= 0.0 ? -1.0 : 1.0;
    drawArrow(painter, startArcPoint, startAngleRad + directionSign * (M_PI / 2.0), m_style, dimColor, arrowSize);
    drawArrow(painter, endArcPoint, endAngleRad - directionSign * (M_PI / 2.0), m_style, dimColor, arrowSize);

    QString text = m_customText.isEmpty()
        ? QString("%1°").arg(QString::number(m_measuredValue, 'f', 2))
        : m_customText;

    const double midAngle = startAngleRad + sweepRad / 2.0;
    QPointF textPoint = getTextAnchor();
    if (!hasCustomTextPosition()) {
        textPoint = QPointF(
        m_centerPoint.x() + (radius + 6.0 / currentScale) * std::cos(midAngle),
        m_centerPoint.y() + (radius + 6.0 / currentScale) * std::sin(midAngle));
        textPoint += QPointF(std::cos(midAngle), std::sin(midAngle)) * (m_style.textGap / currentScale);
    }

    const double tangentAngle = midAngle + (sweepRad >= 0.0 ? M_PI / 2.0 : -M_PI / 2.0);
    const QPointF tangentPoint = textPoint + QPointF(std::cos(tangentAngle), std::sin(tangentAngle));
    const QPointF screenA = worldTransform.map(textPoint);
    const QPointF screenB = worldTransform.map(tangentPoint);
    double textAngle = std::atan2(screenB.y() - screenA.y(), screenB.x() - screenA.x()) * 180.0 / M_PI;
    normalizeReadableScreenAngle(textAngle);

    painter.restore();
    painter.save();
    painter.resetTransform();

    QFont font(m_style.fontFamily);
    font.setPixelSize(std::max(1, static_cast<int>(std::round(m_style.textHeight))));
    painter.setFont(font);
    painter.setPen(isSelected ? Qt::red : m_style.textColor);
    painter.translate(screenA);
    painter.rotate(textAngle);

    QFontMetricsF fm(font);
    const double textWidth = fm.horizontalAdvance(text);
    const double textHeight = fm.height();
    painter.drawText(QRectF(-textWidth / 2.0, -textHeight / 2.0, textWidth, textHeight), Qt::AlignCenter, text);

    painter.restore();
}

QRectF AngularDimensionPrimitive::getBoundingBox() const
{
    const double minX = std::min({m_centerPoint.x(), m_startPoint.x(), m_endPoint.x(), m_arcPoint.x()});
    const double maxX = std::max({m_centerPoint.x(), m_startPoint.x(), m_endPoint.x(), m_arcPoint.x()});
    const double minY = std::min({m_centerPoint.y(), m_startPoint.y(), m_endPoint.y(), m_arcPoint.y()});
    const double maxY = std::max({m_centerPoint.y(), m_startPoint.y(), m_endPoint.y(), m_arcPoint.y()});
    QRectF bbox(QPointF(minX, minY), QPointF(maxX, maxY));
    bbox.adjust(-25.0, -25.0, 25.0, 25.0);
    return bbox;
}

bool AngularDimensionPrimitive::hitTest(const QPointF& point, double tolerance) const
{
    QRectF bbox = getBoundingBox();
    bbox.adjust(-tolerance, -tolerance, tolerance, tolerance);
    return bbox.contains(point);
}

bool AngularDimensionPrimitive::intersects(const QRectF& rect) const
{
    return getBoundingBox().intersects(rect);
}

bool AngularDimensionPrimitive::inside(const QRectF& rect) const
{
    return rect.contains(getBoundingBox());
}

QVector<QPointF> AngularDimensionPrimitive::getSnapPoints() const
{
    return {m_centerPoint, m_startPoint, m_endPoint, m_arcPoint};
}

QPointF AngularDimensionPrimitive::getDefaultTextAnchor() const
{
    const double startAngleRad = std::atan2(m_startPoint.y() - m_centerPoint.y(), m_startPoint.x() - m_centerPoint.x());
    const double endAngleRad = std::atan2(m_endPoint.y() - m_centerPoint.y(), m_endPoint.x() - m_centerPoint.x());
    const double sweepRad = deltaAngle(startAngleRad, endAngleRad);
    const double midAngle = startAngleRad + sweepRad / 2.0;
    const double radius = pointDistance(m_centerPoint, m_arcPoint);
    return QPointF(
        m_centerPoint.x() + (radius + 6.0) * std::cos(midAngle),
        m_centerPoint.y() + (radius + 6.0) * std::sin(midAngle));
}

QVector<QPointF> AngularDimensionPrimitive::getEditGripPoints() const
{
    return {m_startPoint, m_endPoint, m_arcPoint};
}

void AngularDimensionPrimitive::moveGripPoint(int index, const QPointF& newPos)
{
    if (index == 0) m_startPoint = newPos;
    else if (index == 1) m_endPoint = newPos;
    else if (index == 2) m_arcPoint = newPos;
    recalculateValue();
}

void AngularDimensionPrimitive::updateFromAssociation()
{
    QPointF a1, a2, b1, b2;
    if (!edgePoints(m_firstSource, m_firstEdgeIndex, a1, a2)) return;
    if (!edgePoints(m_secondSource, m_secondEdgeIndex, b1, b2)) return;

    QPointF center;
    if (!lineIntersection(a1, a2, b1, b2, center)) return;
    const double radius = pointDistance(m_centerPoint, m_arcPoint);

    QLineF line1(center, a2);
    QLineF line2(center, b2);
    if (line1.length() < 1e-6 || line2.length() < 1e-6) return;
    line1.setLength(radius);
    line2.setLength(radius);

    m_centerPoint = center;
    m_startPoint = line1.p2();
    m_endPoint = line2.p2();
    recalculateValue();
}
