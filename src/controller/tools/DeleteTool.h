#pragma once
#include "BaseCreationTool.h"

class DeleteTool : public BaseCreationTool
{
    Q_OBJECT
public:
    explicit DeleteTool(QObject* parent = nullptr);
    void onMousePress(QMouseEvent* event, Scene* scene, ViewportPanelWidget* viewport) override;
    void onMouseMove(QMouseEvent*, Scene*, ViewportPanelWidget*) override {}; // Не нужен
    void onMouseRelease(QMouseEvent*, Scene*, ViewportPanelWidget*) override {}; // Не нужен
    void reset() override;

signals:
    void primitiveHit(BasePrimitive* primitive);
};
