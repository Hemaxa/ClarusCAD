#pragma once

#include <QObject>

class QMouseEvent;
class QKeyEvent;
class QPainter;
class Scene;
class ViewportWidget;

class BaseTool : public QObject
{
    Q_OBJECT

public:
    explicit BaseTool(QObject* parent = nullptr) : QObject(parent) {}
    virtual ~BaseTool() = default;

    virtual void onMousePress(QMouseEvent* event, Scene* scene, ViewportWidget* viewport) = 0;
    virtual void onMouseMove(QMouseEvent* event, Scene* scene, ViewportWidget* viewport) = 0;
    virtual void onMouseRelease(QMouseEvent* event, Scene* scene, ViewportWidget* viewport) = 0;

    virtual void onKeyPress(QKeyEvent* event) { Q_UNUSED(event); }
    virtual void onPaint(QPainter& painter) { Q_UNUSED(painter); }
};
