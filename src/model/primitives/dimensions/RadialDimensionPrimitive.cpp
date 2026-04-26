// RadialDimensionPrimitive.cpp

#include "RadialDimensionPrimitive.h"
#include "../../../view/managers/LineStyleManager.h"
#include "../../primitives/CirclePrimitive.h"
#include "../../primitives/ArcPrimitive.h"
#include "../../primitives/EllipsePrimitive.h"

#include <algorithm>
#include <cmath>

namespace {
double distance(const QPointF& a, const QPointF& b)
{
    const double dx = b.x() - a.x();
    const double dy = b.y() - a.y();
    return std::sqrt(dx * dx + dy * dy);
}

QPointF normalizedDirection(const QPointF& from, const QPointF& to)
{
    const double len = distance(from, to);
    if (len < 1e-6) {
        return QPointF(1.0, 0.0);
    }
    return QPointF((to.x() - from.x()) / len, (to.y() - from.y()) / len);
}

QPointF projectPointOnRay(const QPointF& point, const QPointF& origin, const QPointF& unitDir, double minDistance)
{
    const QPointF delta = point - origin;
    const double projectedDistance = delta.x() * unitDir.x() + delta.y() * unitDir.y();
    const double distanceOnRay = std::max(minDistance, projectedDistance);
    return origin + unitDir * distanceOnRay;
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
}

void RadialDimensionPrimitive::setDimensionLinePos(const QPointF& pos)
{
    const double radius = distance(m_centerPoint, m_radiusPoint);
    QPointF dir = m_isDiameter
        ? normalizedDirection(m_centerPoint, m_radiusPoint)
        : normalizedDirection(m_centerPoint, m_radiusPoint);
    if (radius < 1e-6 && distance(m_centerPoint, pos) > 1e-6) {
        dir = normalizedDirection(m_centerPoint, pos);
    }

    const double minDistance = (radius > 1e-6) ? radius : 0.0;
    m_dimensionLinePos = projectPointOnRay(pos, m_centerPoint, dir, minDistance);
}

void RadialDimensionPrimitive::recalculateValue()
{
    const double radius = distance(m_centerPoint, m_radiusPoint);
    m_measuredValue = m_isDiameter ? radius * 2.0 : radius;
}

bool RadialDimensionPrimitive::applyMeasuredValueOverride(double value)
{
    if (value <= 0.0) return false;

    const double radius = m_isDiameter ? value / 2.0 : value;
    if (m_associatedPrimitive) {
        switch (m_associatedPrimitive->getType()) {
        case PrimitiveType::Circle: {
            auto* circle = static_cast<CirclePrimitive*>(m_associatedPrimitive);
            circle->setRadius(radius);
            updateFromAssociation();
            return true;
        }
        case PrimitiveType::Arc: {
            auto* arc = static_cast<ArcPrimitive*>(m_associatedPrimitive);
            arc->setRadius(radius);
            updateFromAssociation();
            return true;
        }
        case PrimitiveType::Ellipse: {
            auto* ellipse = static_cast<EllipsePrimitive*>(m_associatedPrimitive);
            const double currentRadius = distance(m_centerPoint, m_radiusPoint);
            if (currentRadius < 1e-6) return false;
            const double scale = radius / currentRadius;
            ellipse->setRadiusX(std::max(0.001, ellipse->getRadiusX() * scale));
            ellipse->setRadiusY(std::max(0.001, ellipse->getRadiusY() * scale));
            updateFromAssociation();
            return true;
        }
        default:
            break;
        }
    }

    QPointF dir = normalizedDirection(m_centerPoint, m_radiusPoint);
    if (m_isDiameter) {
        dir = normalizedDirection(m_centerPoint, m_dimensionLinePos);
    }

    m_radiusPoint = m_centerPoint + QPointF(dir.x() * radius, dir.y() * radius);
    const double leaderDistance = distance(m_centerPoint, m_dimensionLinePos);
    const double targetLeaderDistance = std::max(leaderDistance, radius);
    m_dimensionLinePos = m_centerPoint + QPointF(dir.x() * targetLeaderDistance, dir.y() * targetLeaderDistance);
    recalculateValue();
    return true;
}

void RadialDimensionPrimitive::draw(QPainter& painter, bool isSelected) const
{
    painter.save();

    const QTransform worldTransform = painter.transform();
    double currentScale = std::abs(worldTransform.m11());
    if (currentScale < 0.0001) currentScale = 1.0;

    const double radius = distance(m_centerPoint, m_radiusPoint);
    if (radius < 1e-6) {
        painter.restore();
        return;
    }

    const QPointF dir = m_isDiameter
        ? normalizedDirection(m_centerPoint, m_dimensionLinePos)
        : normalizedDirection(m_centerPoint, m_radiusPoint);
    const QPointF visibleRadiusPoint = m_centerPoint + QPointF(dir.x() * radius, dir.y() * radius);
    const QPointF oppositePoint = m_centerPoint - QPointF(dir.x() * radius, dir.y() * radius);
    QPointF leaderEnd = m_dimensionLinePos;
    const QPointF leaderStart = m_isDiameter ? oppositePoint : m_centerPoint;
    const QPointF lineOrigin = m_isDiameter ? oppositePoint : m_centerPoint;

    const QColor dimColor = isSelected ? Qt::red : m_style.dimensionLineColor;
    if (!m_isDiameter) {
        LineStyleManager::instance().drawLine(painter, m_centerPoint, m_radiusPoint, m_style.extensionLineTypeId,
                                              isSelected ? Qt::red : m_style.extensionLineColor, false);
    }

    QString text = m_customText.isEmpty()
        ? QString("%1%2").arg(m_isDiameter ? QStringLiteral("Ø") : QStringLiteral("R")).arg(QString::number(m_measuredValue, 'f', 2))
        : m_customText;

    QPointF textPoint = getTextAnchor();
    const double angleRad = std::atan2(leaderEnd.y() - visibleRadiusPoint.y(), leaderEnd.x() - visibleRadiusPoint.x());
    if (!hasCustomTextPosition()) {
        textPoint += QPointF(std::cos(angleRad), std::sin(angleRad)) * (m_style.textAlongLineOffset / currentScale);
    }
    QFont font(m_style.fontFamily);
    font.setPixelSize(std::max(1, static_cast<int>(std::round(m_style.textHeight))));
    QFontMetricsF fm(font);
    const double textHalfWidthWorld = fm.horizontalAdvance(text) / (2.0 * currentScale);
    const double textMarginWorld = std::max(2.0, m_style.textGap * 0.25) / currentScale;
    const double anchorDistance = QLineF(lineOrigin, textPoint).length();
    const double leaderDistance = QLineF(lineOrigin, leaderEnd).length();
    const double extendedLeaderDistance = std::max(leaderDistance, anchorDistance + textHalfWidthWorld + textMarginWorld);
    leaderEnd = lineOrigin + dir * extendedLeaderDistance;
    LineStyleManager::instance().drawLine(painter, leaderStart, leaderEnd, m_style.dimensionLineTypeId, dimColor, false);

    const double arrowSize = m_style.arrowSize / currentScale;
    drawArrow(painter, visibleRadiusPoint, std::atan2(m_centerPoint.y() - visibleRadiusPoint.y(), m_centerPoint.x() - visibleRadiusPoint.x()),
              m_style, dimColor, arrowSize);
    if (m_isDiameter) {
        drawArrow(painter, oppositePoint, std::atan2(m_centerPoint.y() - oppositePoint.y(), m_centerPoint.x() - oppositePoint.x()),
                  m_style, dimColor, arrowSize);
    }

    const QPointF screenA = worldTransform.map(visibleRadiusPoint);
    const QPointF screenB = worldTransform.map(leaderEnd);
    double textAngle = std::atan2(screenB.y() - screenA.y(), screenB.x() - screenA.x()) * 180.0 / M_PI;
    normalizeReadableScreenAngle(textAngle);

    painter.restore();
    painter.save();
    painter.resetTransform();

    painter.setFont(font);
    painter.setPen(isSelected ? Qt::red : m_style.textColor);
    painter.translate(worldTransform.map(textPoint));
    painter.rotate(textAngle);
    const double textWidth = fm.horizontalAdvance(text);
    const double textHeight = fm.height();
    painter.drawText(QRectF(-textWidth / 2.0, -textHeight - m_style.textGap, textWidth, textHeight), Qt::AlignCenter, text);

    painter.restore();
}

QRectF RadialDimensionPrimitive::getBoundingBox() const
{
    const double radius = distance(m_centerPoint, m_radiusPoint);
    const QPointF dir = m_isDiameter
        ? normalizedDirection(m_centerPoint, m_dimensionLinePos)
        : normalizedDirection(m_centerPoint, m_radiusPoint);
    const QPointF visibleRadiusPoint = m_centerPoint + QPointF(dir.x() * radius, dir.y() * radius);
    const QPointF oppositePoint = m_centerPoint - QPointF(dir.x() * radius, dir.y() * radius);

    const double minX = std::min({m_centerPoint.x(), m_radiusPoint.x(), visibleRadiusPoint.x(), m_dimensionLinePos.x(), oppositePoint.x()});
    const double maxX = std::max({m_centerPoint.x(), m_radiusPoint.x(), visibleRadiusPoint.x(), m_dimensionLinePos.x(), oppositePoint.x()});
    const double minY = std::min({m_centerPoint.y(), m_radiusPoint.y(), visibleRadiusPoint.y(), m_dimensionLinePos.y(), oppositePoint.y()});
    const double maxY = std::max({m_centerPoint.y(), m_radiusPoint.y(), visibleRadiusPoint.y(), m_dimensionLinePos.y(), oppositePoint.y()});

    QRectF bbox(QPointF(minX, minY), QPointF(maxX, maxY));
    bbox.adjust(-25.0, -25.0, 25.0, 25.0);
    return bbox;
}

bool RadialDimensionPrimitive::hitTest(const QPointF& point, double tolerance) const
{
    QRectF bbox = getBoundingBox();
    bbox.adjust(-tolerance, -tolerance, tolerance, tolerance);
    return bbox.contains(point);
}

bool RadialDimensionPrimitive::intersects(const QRectF& rect) const
{
    return getBoundingBox().intersects(rect);
}

bool RadialDimensionPrimitive::inside(const QRectF& rect) const
{
    return rect.contains(getBoundingBox());
}

QVector<QPointF> RadialDimensionPrimitive::getSnapPoints() const
{
    return {m_centerPoint, m_radiusPoint, m_dimensionLinePos};
}

QPointF RadialDimensionPrimitive::getDefaultTextAnchor() const
{
    const double radius = distance(m_centerPoint, m_radiusPoint);
    const QPointF dir = m_isDiameter
        ? normalizedDirection(m_centerPoint, m_dimensionLinePos)
        : normalizedDirection(m_centerPoint, m_radiusPoint);
    const QPointF visibleRadiusPoint = m_centerPoint + QPointF(dir.x() * radius, dir.y() * radius);
    return QPointF((visibleRadiusPoint.x() + m_dimensionLinePos.x()) / 2.0,
                   (visibleRadiusPoint.y() + m_dimensionLinePos.y()) / 2.0);
}

QPointF RadialDimensionPrimitive::constrainTextAnchor(const QPointF& pos) const
{
    const double radius = distance(m_centerPoint, m_radiusPoint);
    QPointF dir = normalizedDirection(m_centerPoint, m_radiusPoint);
    if (radius < 1e-6) {
        dir = normalizedDirection(m_centerPoint, pos);
    }

    const QPointF origin = m_isDiameter ? (m_centerPoint - dir * radius) : m_centerPoint;
    return projectPointOnRay(pos, origin, dir, 0.0);
}

QVector<QPointF> RadialDimensionPrimitive::getEditGripPoints() const
{
    return {m_centerPoint, m_radiusPoint, m_dimensionLinePos};
}

void RadialDimensionPrimitive::moveGripPoint(int index, const QPointF& newPos)
{
    if (index == 0) m_centerPoint = newPos;
    else if (index == 1) m_radiusPoint = newPos;
    else if (index == 2) setDimensionLinePos(newPos);
    recalculateValue();
}

void RadialDimensionPrimitive::updateFromAssociation()
{
    if (!m_associatedPrimitive) return;

    switch (m_associatedPrimitive->getType()) {
    case PrimitiveType::Circle: {
        auto* c = static_cast<CirclePrimitive*>(m_associatedPrimitive);
        m_centerPoint = QPointF(c->getCenter().getX(), c->getCenter().getY());
        m_radiusPoint = QPointF(m_centerPoint.x() + c->getRadius() * std::cos(m_associationAngle),
                                m_centerPoint.y() + c->getRadius() * std::sin(m_associationAngle));
        break;
    }
    case PrimitiveType::Arc: {
        auto* a = static_cast<ArcPrimitive*>(m_associatedPrimitive);
        m_centerPoint = QPointF(a->getCenter().getX(), a->getCenter().getY());
        m_radiusPoint = QPointF(m_centerPoint.x() + a->getRadius() * std::cos(m_associationAngle),
                                m_centerPoint.y() + a->getRadius() * std::sin(m_associationAngle));
        break;
    }
    case PrimitiveType::Ellipse: {
        auto* e = static_cast<EllipsePrimitive*>(m_associatedPrimitive);
        m_centerPoint = QPointF(e->getCenter().getX(), e->getCenter().getY());
        QTransform t;
        t.translate(m_centerPoint.x(), m_centerPoint.y());
        t.rotate(e->getRotation());
        m_radiusPoint = t.map(QPointF(e->getRadiusX() * std::cos(m_associationAngle),
                                      e->getRadiusY() * std::sin(m_associationAngle)));
        break;
    }
    default:
        break;
    }
    recalculateValue();
}
