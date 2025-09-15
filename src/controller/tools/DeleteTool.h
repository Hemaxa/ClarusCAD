#pragma once

#include "BaseCreationTool.h"

class DeleteTool : public BaseCreationTool
{
    Q_OBJECT

public:
    explicit DeleteTool(QObject* parent = nullptr);

    void onMousePress(QMouseEvent* event, Scene* scene, ViewportPanelWidget* viewport) override;
    void onMouseMove(QMouseEvent* event, Scene* scene, ViewportPanelWidget* viewport) override;
    void onMouseRelease(QMouseEvent* event, Scene* scene, ViewportPanelWidget* viewport) override;

    void reset() override;
};
