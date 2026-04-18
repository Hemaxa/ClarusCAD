// AngularDimensionCreationTool.h

#pragma once

#include "../../BaseCreationTool.h"
#include "../../../model/primitives/dimensions/AngularDimensionPrimitive.h"

#include <memory>

class BasePrimitive;

class AngularDimensionCreationTool : public BaseCreationTool {
    Q_OBJECT
public:
    explicit AngularDimensionCreationTool(QObject* parent = nullptr);

    void onMousePress(QMouseEvent* event, Scene* scene, ViewportPanelWidget* viewport) override;
    void onMouseMove(QMouseEvent* event, Scene* scene, ViewportPanelWidget* viewport) override;
    void onMouseRelease(QMouseEvent* event, Scene* scene, ViewportPanelWidget* viewport) override;
    void reset() override;
    void onPaint(QPainter& painter) override;

    void setColor(const QColor& color) override;
    QColor getColor() const override;

signals:
    void dimensionDataReady(AngularDimensionPrimitive* dim);

private:
    BasePrimitive* m_firstEdgeSource = nullptr;
    int m_firstEdgeIndex = -1;
    bool m_hasFirstEdge = false;
    QPointF m_firstEdgeA;
    QPointF m_firstEdgeB;
    int m_state = 0;
    std::unique_ptr<AngularDimensionPrimitive> m_previewDimension;
    QColor m_currentColor = Qt::white;
};
