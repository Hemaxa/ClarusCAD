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
}

void RadialDimensionPrimitive::recalculateValue()
{
    const double radius = distance(m_centerPoint, m_radiusPoint);
    m_measuredValue = m_isDiameter ? radius * 2.0 : radius;
}

void RadialDimensionPrimitive::draw(QPainter& painter, bool isSelected) const
{
    painter.save();

    double currentScale = std::abs(painter.transform().m11());
    if (currentScale < 0.0001) currentScale = 1.0;

    const QPointF dir = normalizedDirection(m_centerPoint, m_radiusPoint);
    const double radius = distance(m_centerPoint, m_radiusPoint);
    if (radius < 1e-6) {
        painter.restore();
        return;
    }

    const QPointF oppositePoint = m_centerPoint - QPointF(dir.x() * radius, dir.y() * radius);
    const QPointF leaderEnd = m_dimensionLinePos;
    const QPointF leaderStart = m_isDiameter ? oppositePoint : m_centerPoint;

    const QColor dimColor = isSelected ? Qt::red : m_style.dimensionLineColor;
    LineStyleManager::instance().drawLine(painter, leaderStart, leaderEnd, m_style.dimensionLineTypeId, dimColor, false);
    if (!m_isDiameter) {
        LineStyleManager::instance().drawLine(painter, m_centerPoint, m_radiusPoint, m_style.extensionLineTypeId,
                                              isSelected ? Qt::red : m_style.extensionLineColor, false);
    }

    const double arrowSize = m_style.arrowSize / currentScale;
    drawArrow(painter, m_radiusPoint, std::atan2(m_centerPoint.y() - m_radiusPoint.y(), m_centerPoint.x() - m_radiusPoint.x()),
              m_style, dimColor, arrowSize);
    if (m_isDiameter) {
        drawArrow(painter, oppositePoint, std::atan2(m_centerPoint.y() - oppositePoint.y(), m_centerPoint.x() - oppositePoint.x()),
                  m_style, dimColor, arrowSize);
    }

    QString text = m_customText.isEmpty()
        ? QString("%1%2").arg(m_isDiameter ? QStringLiteral("Ø") : QStringLiteral("R")).arg(QString::number(m_measuredValue, 'f', 2))
        : m_customText;

    QFont font(m_style.fontFamily);
    font.setPointSizeF(m_style.textHeight / currentScale);
    painter.setFont(font);
    painter.setPen(isSelected ? Qt::red : m_style.textColor);

    QPointF textPoint = getTextAnchor();
    const double angleDeg = std::atan2(leaderEnd.y() - m_radiusPoint.y(), leaderEnd.x() - m_radiusPoint.x()) * 180.0 / M_PI;
    if (!hasCustomTextPosition()) {
        textPoint += QPointF(std::cos(angleDeg * M_PI / 180.0), std::sin(angleDeg * M_PI / 180.0)) * (m_style.textAlongLineOffset / currentScale);
    }
    double textAngle = angleDeg;
    if (textAngle > 90.0) textAngle -= 180.0;
    if (textAngle < -90.0) textAngle += 180.0;

    painter.translate(textPoint);
    painter.rotate(textAngle);
    QFontMetricsF fm(font);
    const double textWidth = fm.horizontalAdvance(text);
    const double textHeight = fm.height();
    painter.drawText(QRectF(-textWidth / 2.0, -textHeight - m_style.textGap / currentScale, textWidth, textHeight), Qt::AlignCenter, text);

    painter.restore();
}

QRectF RadialDimensionPrimitive::getBoundingBox() const
{
    const QPointF dir = normalizedDirection(m_centerPoint, m_radiusPoint);
    const double radius = distance(m_centerPoint, m_radiusPoint);
    const QPointF oppositePoint = m_centerPoint - QPointF(dir.x() * radius, dir.y() * radius);

    const double minX = std::min({m_centerPoint.x(), m_radiusPoint.x(), m_dimensionLinePos.x(), oppositePoint.x()});
    const double maxX = std::max({m_centerPoint.x(), m_radiusPoint.x(), m_dimensionLinePos.x(), oppositePoint.x()});
    const double minY = std::min({m_centerPoint.y(), m_radiusPoint.y(), m_dimensionLinePos.y(), oppositePoint.y()});
    const double maxY = std::max({m_centerPoint.y(), m_radiusPoint.y(), m_dimensionLinePos.y(), oppositePoint.y()});

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
    return QPointF((m_radiusPoint.x() + m_dimensionLinePos.x()) / 2.0,
                   (m_radiusPoint.y() + m_dimensionLinePos.y()) / 2.0);
}

QVector<QPointF> RadialDimensionPrimitive::getEditGripPoints() const
{
    return {m_centerPoint, m_radiusPoint, m_dimensionLinePos};
}

void RadialDimensionPrimitive::moveGripPoint(int index, const QPointF& newPos)
{
    if (index == 0) m_centerPoint = newPos;
    else if (index == 1) m_radiusPoint = newPos;
    else if (index == 2) m_dimensionLinePos = newPos;
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
