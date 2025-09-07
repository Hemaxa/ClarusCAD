#pragma once

#include "BaseTool.h"
#include "Point.h"

class LineCreationTool : public BaseTool
{
    Q_OBJECT
public:
    explicit LineCreationTool(QObject* parent = nullptr);

    void onMousePress(QMouseEvent* event, Scene* scene, ViewportWidget* viewport) override;
    void onMouseMove(QMouseEvent* event, Scene* scene, ViewportWidget* viewport) override;
    void onMouseRelease(QMouseEvent* event, Scene* scene, ViewportWidget* viewport) override;
    void onPaint(QPainter& painter) override;

private:
    enum class State {
        Idle,
        WaitingForSecondPoint
    };

    State m_currentState;
    Point m_firstPoint;
    Point m_currentMousePos;
};
