//BaseCreationTool — базовый класс для всех инструментов в приложении

#pragma once

#include <QObject>
#include <QColor>

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

    //виртуальный метод очистки инструмента
    virtual void reset() {}

    //виртуальный метод для установки цвета
    virtual void setColor(const QColor& color) { Q_UNUSED(color); }

    //виртуальный метод для получения установленного цвета
    virtual QColor getColor() const { return Qt::white; }

    //виртуальный вспомогательный метод для дополнительной геометрии
    virtual void onPaint(QPainter& painter) { Q_UNUSED(painter); }
};
