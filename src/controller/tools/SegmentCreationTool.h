#pragma once

#include "BaseTool.h"
#include "PointCreationPrimitive.h"

class SegmentCreationTool : public BaseTool
{
    Q_OBJECT

public:
    explicit SegmentCreationTool(QObject* parent = nullptr);

    void onMousePress(QMouseEvent* event, Scene* scene, ViewportPanelWidget* viewport) override;
    void onMouseMove(QMouseEvent* event, Scene* scene, ViewportPanelWidget* viewport) override;
    void onMouseRelease(QMouseEvent* event, Scene* scene, ViewportPanelWidget* viewport) override;
    void onPaint(QPainter& painter) override;

private:
    enum class State { Idle, WaitingForSecondPoint };
    State m_currentState;
    PointCreationPrimitive m_firstPoint;
    PointCreationPrimitive m_currentMousePos;
};
