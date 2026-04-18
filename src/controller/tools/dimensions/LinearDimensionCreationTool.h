// LinearDimensionCreationTool.h
#pragma once

#include "../../BaseCreationTool.h"
#include "../../../model/primitives/dimensions/LinearDimensionPrimitive.h"
#include <memory>

class LinearDimensionCreationTool : public BaseCreationTool {
    Q_OBJECT
public:
    explicit LinearDimensionCreationTool(QObject* parent = nullptr);
    virtual ~LinearDimensionCreationTool() = default;

    void onMousePress(QMouseEvent* event, Scene* scene, ViewportPanelWidget* viewport) override;
    void onMouseMove(QMouseEvent* event, Scene* scene, ViewportPanelWidget* viewport) override;
    void onMouseRelease(QMouseEvent* event, Scene* scene, ViewportPanelWidget* viewport) override;
    void reset() override;
    void onPaint(QPainter& painter) override;

    void setColor(const QColor& color) override;
    QColor getColor() const override;
    void setMode(LinearDimensionMode mode) { m_mode = mode; if (m_previewDimension) m_previewDimension->setMode(mode); }
    LinearDimensionMode getMode() const { return m_mode; }

signals:
    void dimensionDataReady(LinearDimensionPrimitive* dim); // <--- ДОБАВИТЬ БЛОК СИГНАЛОВ

private:
    int m_state = 0; // 0: Ожидание первой точки, 1: Ожидание второй точки, 2: Ожидание положения линии
    std::unique_ptr<LinearDimensionPrimitive> m_previewDimension;
    QColor m_currentColor = Qt::white;
    LinearDimensionMode m_mode = LinearDimensionMode::Aligned;
};
