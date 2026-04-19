#include "RadialDimensionCreationTool.h"

#include "../../../model/Scene.h"
#include "../../../model/primitives/CirclePrimitive.h"
#include "../../../model/primitives/ArcPrimitive.h"
#include "../../../model/primitives/EllipsePrimitive.h"
#include "../../../view/managers/ObjectBindingManager.h"
#include "../../../view/managers/SettingsManager.h"
#include "../../../view/managers/SnapManager.h"
#include "../../../view/panels/ViewportPanelWidget.h"

#include <QMouseEvent>
#include <cmath>

namespace {
bool circularPrimitiveCenter(BasePrimitive* primitive, QPointF& center)
{
    if (!primitive) return false;
    switch (primitive->getType()) {
    case PrimitiveType::Circle: {
        auto* c = static_cast<CirclePrimitive*>(primitive);
        center = QPointF(c->getCenter().getX(), c->getCenter().getY());
        return true;
    }
    case PrimitiveType::Arc: {
        auto* a = static_cast<ArcPrimitive*>(primitive);
        center = QPointF(a->getCenter().getX(), a->getCenter().getY());
        return true;
    }
    case PrimitiveType::Ellipse: {
        auto* e = static_cast<EllipsePrimitive*>(primitive);
        center = QPointF(e->getCenter().getX(), e->getCenter().getY());
        return true;
    }
    default:
        return false;
    }
}
}

RadialDimensionCreationTool::RadialDimensionCreationTool(QObject* parent)
    : BaseCreationTool(parent)
{
}

void RadialDimensionCreationTool::onMousePress(QMouseEvent* event, Scene* scene, ViewportPanelWidget* viewport)
{
    Q_UNUSED(scene);
    if (event->button() == Qt::LeftButton) {
        const SnapPoint snap = viewport->getSnapPoint(event->position());
        const QPointF pos = snap.position;
        if (m_state == 0) {
            m_previewDimension = std::make_unique<RadialDimensionPrimitive>();
            m_previewDimension->setDiameterMode(m_isDiameterMode);

            DimensionStyle style = SettingsManager::instance().getDefaultDimensionStyle();
            style.dimensionLineColor = m_currentColor;
            style.extensionLineColor = m_currentColor;
            style.textColor = m_currentColor;
            m_previewDimension->setStyle(style);

            QPointF sourceCenter;
            if (snap.type != SnapType::Center && circularPrimitiveCenter(snap.source, sourceCenter)) {
                m_previewDimension->setCenterPoint(sourceCenter);
                m_previewDimension->setRadiusPoint(pos);
                m_previewDimension->setDimensionLinePos(pos);
                m_previewDimension->setAssociatedPrimitive(
                    snap.source,
                    std::atan2(pos.y() - sourceCenter.y(), pos.x() - sourceCenter.x()));
                m_previewDimension->recalculateValue();
                ObjectBindingManager::instance().setPrimitiveSnap(false);
                m_state = 2;
            } else {
                m_previewDimension->setCenterPoint(pos);
                m_previewDimension->setRadiusPoint(pos);
                m_previewDimension->setDimensionLinePos(pos);
                SnapManager::instance().setBasePoint(pos);
                m_state = 1;
            }
        } else if (m_state == 1) {
            m_previewDimension->setRadiusPoint(pos);
            QPointF sourceCenter;
            if (snap.type != SnapType::Center && circularPrimitiveCenter(snap.source, sourceCenter)) {
                m_previewDimension->setCenterPoint(sourceCenter);
                m_previewDimension->setRadiusPoint(pos);
                m_previewDimension->setAssociatedPrimitive(
                    snap.source,
                    std::atan2(pos.y() - sourceCenter.y(), pos.x() - sourceCenter.x()));
            }
            m_previewDimension->recalculateValue();
            SnapManager::instance().clearBasePoint();
            ObjectBindingManager::instance().setPrimitiveSnap(false);
            m_state = 2;
        } else if (m_state == 2) {
            m_previewDimension->setDimensionLinePos(pos);
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

void RadialDimensionCreationTool::onMouseMove(QMouseEvent* event, Scene* scene, ViewportPanelWidget* viewport)
{
    Q_UNUSED(scene);
    if (!m_previewDimension) return;

    const QPointF pos = viewport->getSnappedPoint(event->position());
    if (m_state == 1) {
        m_previewDimension->setRadiusPoint(pos);
        m_previewDimension->setDimensionLinePos(pos);
        m_previewDimension->recalculateValue();
    } else if (m_state == 2) {
        m_previewDimension->setDimensionLinePos(pos);
        m_previewDimension->recalculateValue();
    }
    viewport->update();
}

void RadialDimensionCreationTool::onMouseRelease(QMouseEvent* event, Scene* scene, ViewportPanelWidget* viewport)
{
    Q_UNUSED(event);
    Q_UNUSED(scene);
    Q_UNUSED(viewport);
}

void RadialDimensionCreationTool::reset()
{
    m_state = 0;
    m_previewDimension.reset();
    SnapManager::instance().clearBasePoint();
    ObjectBindingManager::instance().setPrimitiveSnap(true);
}

void RadialDimensionCreationTool::onPaint(QPainter& painter)
{
    if (m_previewDimension && (m_state == 1 || m_state == 2)) {
        m_previewDimension->draw(painter, false);
    }
}

void RadialDimensionCreationTool::setColor(const QColor& color)
{
    m_currentColor = color;
    if (m_previewDimension) {
        m_previewDimension->setColor(color);
    }
}

QColor RadialDimensionCreationTool::getColor() const
{
    return m_currentColor;
}

void RadialDimensionCreationTool::setDiameterMode(bool isDiameter)
{
    m_isDiameterMode = isDiameter;
    if (m_previewDimension) {
        m_previewDimension->setDiameterMode(isDiameter);
    }
}
