// LinearDimensionCreationTool.cpp
#include "LinearDimensionCreationTool.h"
#include "../../../view/panels/ViewportPanelWidget.h"
#include "../../../view/managers/SnapManager.h"
#include "../../../view/managers/ObjectBindingManager.h"
#include "../../../view/managers/SettingsManager.h"
#include "../../../model/Scene.h"
#include "../../../model/primitives/PolygonPrimitive.h"
#include <QMouseEvent>
#include <QWidget>
#include <algorithm>
#include <cmath>

namespace {
LinearDimensionPrimitive::Attachment makeAttachment(const SnapPoint& snap, const QPointF& pos)
{
    LinearDimensionPrimitive::Attachment a;
    a.source = snap.source;
    a.snapType = snap.type;
    a.fallback = pos;
    if (!snap.source) return a;

    switch (snap.source->getType()) {
    case PrimitiveType::Segment: {
        auto points = snap.source->getSnapPoints();
        if (snap.type == SnapType::Endpoint) {
            a.index = (QLineF(pos, points[0]).length() < QLineF(pos, points[1]).length()) ? 0 : 1;
        } else if (snap.type == SnapType::Nearest) {
            QPointF p1 = points[0], p2 = points[1];
            const double lenSq = std::pow(p2.x()-p1.x(),2)+std::pow(p2.y()-p1.y(),2);
            if (lenSq > 1e-12) a.param = ((pos.x()-p1.x())*(p2.x()-p1.x()) + (pos.y()-p1.y())*(p2.y()-p1.y())) / lenSq;
        }
        break;
    }
    case PrimitiveType::Circle:
    case PrimitiveType::Arc:
    case PrimitiveType::Ellipse: {
        QPointF center = snap.source->getSnapPoints().first();
        a.param = std::atan2(pos.y() - center.y(), pos.x() - center.x());
        break;
    }
    case PrimitiveType::Rectangle: {
        auto pts = snap.source->getSnapPoints();
        if (snap.type == SnapType::Endpoint) {
            double best = 1e18;
            for (int i = 0; i < 4; ++i) {
                double d = QLineF(pos, pts[1 + i]).length();
                if (d < best) { best = d; a.index = i; }
            }
        } else if (snap.type == SnapType::Midpoint) {
            double best = 1e18;
            for (int i = 0; i < 4; ++i) {
                double d = QLineF(pos, pts[5 + i]).length();
                if (d < best) { best = d; a.index = i; }
            }
        } else if (snap.type == SnapType::Nearest) {
            double best = 1e18;
            for (int i = 0; i < 4; ++i) {
                QPointF p1 = pts[1 + i], p2 = pts[1 + ((i + 1) % 4)];
                QPointF cp = pos;
                const double lenSq = std::pow(p2.x()-p1.x(),2)+std::pow(p2.y()-p1.y(),2);
                if (lenSq < 1e-12) continue;
                double t = ((pos.x()-p1.x())*(p2.x()-p1.x()) + (pos.y()-p1.y())*(p2.y()-p1.y())) / lenSq;
                t = std::clamp(t, 0.0, 1.0);
                cp = p1 + (p2-p1) * t;
                double d = QLineF(pos, cp).length();
                if (d < best) { best = d; a.index = i; a.param = t; }
            }
        }
        break;
    }
    case PrimitiveType::Polygon: {
        auto* poly = static_cast<PolygonPrimitive*>(snap.source);
        auto verts = poly->getVertices();
        if (snap.type == SnapType::Endpoint) {
            double best = 1e18;
            for (int i = 0; i < verts.size(); ++i) {
                double d = QLineF(pos, verts[i]).length();
                if (d < best) { best = d; a.index = i; }
            }
        } else if (snap.type == SnapType::Nearest) {
            double best = 1e18;
            for (int i = 0; i < verts.size(); ++i) {
                QPointF p1 = verts[i], p2 = verts[(i + 1) % verts.size()];
                const double lenSq = std::pow(p2.x()-p1.x(),2)+std::pow(p2.y()-p1.y(),2);
                if (lenSq < 1e-12) continue;
                double t = ((pos.x()-p1.x())*(p2.x()-p1.x()) + (pos.y()-p1.y())*(p2.y()-p1.y())) / lenSq;
                t = std::clamp(t, 0.0, 1.0);
                QPointF cp = p1 + (p2-p1) * t;
                double d = QLineF(pos, cp).length();
                if (d < best) { best = d; a.index = i; a.param = t; }
            }
        }
        break;
    }
    default:
        break;
    }
    return a;
}
}

LinearDimensionCreationTool::LinearDimensionCreationTool(QObject* parent) 
    : BaseCreationTool(parent), m_state(0) 
{
}

void LinearDimensionCreationTool::onMousePress(QMouseEvent* event, Scene* scene, ViewportPanelWidget* viewport) {
    if (event->button() == Qt::LeftButton) {
        if (m_state == 0) {
            // Первая точка (привязки включены)
            SnapPoint snap = viewport->getSnapPoint(event->position());
            QPointF pos = snap.position;
            
            m_previewDimension = std::make_unique<LinearDimensionPrimitive>();
            m_previewDimension->setStartPoint(pos);
            m_previewDimension->setEndPoint(pos);
            m_previewDimension->setDimensionLinePos(pos);
            m_previewDimension->setMode(m_mode);
            m_previewDimension->setStartAttachment(makeAttachment(snap, pos));
            
            DimensionStyle style = SettingsManager::instance().getDefaultDimensionStyle();
            style.dimensionLineColor = m_currentColor;
            style.extensionLineColor = m_currentColor;
            style.textColor = m_currentColor;
            m_previewDimension->setStyle(style);
            
            SnapManager::instance().setBasePoint(pos);
            
            m_state = 1;
            viewport->update();
        } else if (m_state == 1) {
            // Вторая точка (привязки включены)
            SnapPoint snap = viewport->getSnapPoint(event->position());
            QPointF pos = snap.position;
            
            m_previewDimension->setEndPoint(pos);
            m_previewDimension->setEndAttachment(makeAttachment(snap, pos));
            m_previewDimension->recalculateValue();
            
            SnapManager::instance().clearBasePoint();
            // Отключаем привязки к объектам, чтобы линия не "прыгала", пока мы ее позиционируем
            ObjectBindingManager::instance().setPrimitiveSnap(false);
            
            m_state = 2;
            viewport->update();
        } else if (m_state == 2) {
            // Положение размерной линии
            QPointF pos = viewport->getSnappedPoint(event->position());
            
            m_previewDimension->setDimensionLinePos(pos);
            m_previewDimension->recalculateValue();
            
            // Добавляем на сцену
            if (m_previewDimension) {
                emit dimensionDataReady(m_previewDimension.release());
            }
            
            // Восстанавливаем привязки
            ObjectBindingManager::instance().setPrimitiveSnap(true);
            
            m_state = 0;
            viewport->update();
        }
    } else if (event->button() == Qt::RightButton) {
        // Отмена
        reset();
        viewport->update();
    }
}

void LinearDimensionCreationTool::onMouseMove(QMouseEvent* event, Scene* scene, ViewportPanelWidget* viewport) {
    if (m_state == 1) {
        QPointF pos = viewport->getSnappedPoint(event->position());
        m_previewDimension->setEndPoint(pos);
        // Пока не выбрали 3-ю точку, будем держать размерную линию там же, где 2-я точка (или посередине)
        m_previewDimension->setDimensionLinePos(pos); 
        m_previewDimension->recalculateValue();
        viewport->update();
    } else if (m_state == 2) {
        QPointF pos = viewport->getSnappedPoint(event->position());
        m_previewDimension->setDimensionLinePos(pos);
        m_previewDimension->recalculateValue();
        viewport->update();
    }
}

void LinearDimensionCreationTool::onMouseRelease(QMouseEvent* event, Scene* scene, ViewportPanelWidget* viewport) {
    Q_UNUSED(event);
    Q_UNUSED(scene);
    Q_UNUSED(viewport);
}

void LinearDimensionCreationTool::reset() {
    m_state = 0;
    m_previewDimension.reset();
    SnapManager::instance().clearBasePoint();
    ObjectBindingManager::instance().setPrimitiveSnap(true);
}

void LinearDimensionCreationTool::onPaint(QPainter& painter) {
    if (m_previewDimension && (m_state == 1 || m_state == 2)) {
        // Рисуем сам размер (предпросмотр)
        m_previewDimension->draw(painter, false);
        
        // Отрисовка "жирных маркеров" для уже установленных точек
        painter.save();
        painter.setBrush(m_currentColor);
        painter.setPen(QPen(Qt::white, 2.0));
        
        double currentScale = painter.transform().m11();
        if (std::abs(currentScale) < 0.0001) currentScale = 1.0;
        
        // Маркер первой точки
        QPointF sp = m_previewDimension->getStartPoint();
        painter.drawEllipse(sp, 6.0 / currentScale, 6.0 / currentScale);
        
        // Маркер второй точки, если мы в стейте 2
        if (m_state == 2) {
            QPointF ep = m_previewDimension->getEndPoint();
            painter.drawEllipse(ep, 6.0 / currentScale, 6.0 / currentScale);
        }
        
        painter.restore();
    }
}

void LinearDimensionCreationTool::setColor(const QColor& color) {
    m_currentColor = color;
    if (m_previewDimension) {
        DimensionStyle style = m_previewDimension->getStyle();
        style.dimensionLineColor = color;
        m_previewDimension->setStyle(style);
    }
}

QColor LinearDimensionCreationTool::getColor() const {
    return m_currentColor;
}
