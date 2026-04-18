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
            m_previewDimension->setCenterPoint(pos);
            m_previewDimension->setRadiusPoint(pos);
            m_previewDimension->setDimensionLinePos(pos);
            m_previewDimension->setDiameterMode(m_isDiameterMode);

            DimensionStyle style = SettingsManager::instance().getDefaultDimensionStyle();
            style.dimensionLineColor = m_currentColor;
            style.extensionLineColor = m_currentColor;
            style.textColor = m_currentColor;
            m_previewDimension->setStyle(style);

            SnapManager::instance().setBasePoint(pos);
            m_state = 1;
        } else if (m_state == 1) {
            m_previewDimension->setRadiusPoint(pos);
            if (snap.source && (snap.source->getType() == PrimitiveType::Circle
                                || snap.source->getType() == PrimitiveType::Arc
                                || snap.source->getType() == PrimitiveType::Ellipse)) {
                QPointF center = m_previewDimension->getCenterPoint();
                m_previewDimension->setAssociatedPrimitive(snap.source, std::atan2(pos.y() - center.y(), pos.x() - center.x()));
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
