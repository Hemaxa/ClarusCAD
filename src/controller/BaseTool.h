#pragma once

#include <QObject>
#include <memory>

class QMouseEvent;
class QKeyEvent;
class QPainter;
class Scene;
class ViewportPanelWidget;
class BasePrimitive;

class BaseTool : public QObject
{
    Q_OBJECT

public:
    explicit BaseTool(QObject* parent = nullptr) : QObject(parent) {}
    virtual ~BaseTool() = default;

    virtual void onMousePress(QMouseEvent* event, Scene* scene, ViewportPanelWidget* viewport) = 0;
    virtual void onMouseMove(QMouseEvent* event, Scene* scene, ViewportPanelWidget* viewport) = 0;
    virtual void onMouseRelease(QMouseEvent* event, Scene* scene, ViewportPanelWidget* viewport) = 0;
    virtual void onPaint(QPainter& painter) { Q_UNUSED(painter); }

signals:
    void primitiveCreated(BasePrimitive* primitive);
};
