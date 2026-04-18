// LinearDimensionCreationTool.cpp
#include "LinearDimensionCreationTool.h"
#include "../../../view/panels/ViewportPanelWidget.h"
#include "../../../view/managers/SnapManager.h"
#include "../../../view/managers/ObjectBindingManager.h"
#include "../../../model/Scene.h"
#include <QMouseEvent>
#include <QWidget>

LinearDimensionCreationTool::LinearDimensionCreationTool(QObject* parent) 
    : BaseCreationTool(parent), m_state(0) 
{
}

void LinearDimensionCreationTool::onMousePress(QMouseEvent* event, Scene* scene, ViewportPanelWidget* viewport) {
    if (event->button() == Qt::LeftButton) {
        if (m_state == 0) {
            // Первая точка (привязки включены)
            QPointF pos = viewport->getSnappedPoint(event->position());
            
            m_previewDimension = std::make_unique<LinearDimensionPrimitive>();
            m_previewDimension->setStartPoint(pos);
            m_previewDimension->setEndPoint(pos);
            m_previewDimension->setDimensionLinePos(pos);
            
            DimensionStyle style;
            style.dimensionLineColor = m_currentColor;
            style.textColor = Qt::white; // Текст пусть будет белым
            m_previewDimension->setStyle(style);
            
            SnapManager::instance().setBasePoint(pos);
            
            m_state = 1;
            viewport->update();
        } else if (m_state == 1) {
            // Вторая точка (привязки включены)
            QPointF pos = viewport->getSnappedPoint(event->position());
            
            m_previewDimension->setEndPoint(pos);
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
