//BaseCreationTool — базовый класс для всех инструментов в приложении

#pragma once

#include <QObject>

class QMouseEvent;
class QKeyEvent;
class QPainter;
class Scene;
class ViewportPanelWidget;
class BasePrimitive;

class BaseCreationTool : public QObject
{
    Q_OBJECT

public:
    //конструктор и виртуальный деструктор
    explicit BaseCreationTool(QObject* parent = nullptr) : QObject(parent) {}
    virtual ~BaseCreationTool() = default;

    //виртальные методы действий мыши
    virtual void onMousePress(QMouseEvent* event, Scene* scene, ViewportPanelWidget* viewport) = 0;
    virtual void onMouseMove(QMouseEvent* event, Scene* scene, ViewportPanelWidget* viewport) = 0;
    virtual void onMouseRelease(QMouseEvent* event, Scene* scene, ViewportPanelWidget* viewport) = 0;

    //вспомогательный метод для дополнительной геометрии
    virtual void onPaint(QPainter& painter) { Q_UNUSED(painter); }
};
