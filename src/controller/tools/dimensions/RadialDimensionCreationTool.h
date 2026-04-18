// RadialDimensionCreationTool.h

#pragma once

#include "../../BaseCreationTool.h"
#include "../../../model/primitives/dimensions/RadialDimensionPrimitive.h"

#include <memory>

class RadialDimensionCreationTool : public BaseCreationTool {
    Q_OBJECT
public:
    explicit RadialDimensionCreationTool(QObject* parent = nullptr);

    void onMousePress(QMouseEvent* event, Scene* scene, ViewportPanelWidget* viewport) override;
    void onMouseMove(QMouseEvent* event, Scene* scene, ViewportPanelWidget* viewport) override;
    void onMouseRelease(QMouseEvent* event, Scene* scene, ViewportPanelWidget* viewport) override;
    void reset() override;
    void onPaint(QPainter& painter) override;

    void setColor(const QColor& color) override;
    QColor getColor() const override;

    void setDiameterMode(bool isDiameter);
    bool isDiameterMode() const { return m_isDiameterMode; }

signals:
    void dimensionDataReady(RadialDimensionPrimitive* dim);

private:
    int m_state = 0;
    std::unique_ptr<RadialDimensionPrimitive> m_previewDimension;
    QColor m_currentColor = Qt::white;
    bool m_isDiameterMode = false;
};
