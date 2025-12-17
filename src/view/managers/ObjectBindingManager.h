//ObjectBindingManager - менеджер, отвечающий за привязки к сетке и объектам

#pragma once

#include <QObject>
#include <QPointF>

class Scene;
class BasePrimitive;

class ObjectBindingManager : public QObject
{
    Q_OBJECT

public:
    //доступ к единственному экземпляру
    static ObjectBindingManager& instance();

    //основной метод: возвращает скорректированную точку (к сетке или объекту)
    //excludePrimitive - примитив, который нужно игнорировать (например, тот, который сейчас перемещаем)
    QPointF getSnappedPoint(const QPointF& currentPos, const Scene* scene, double zoomFactor, BasePrimitive* excludePrimitive = nullptr) const;

    //методы управления состоянием привязок
    void setGridSnap(bool enabled);
    bool isGridSnapEnabled() const;

    void setPrimitiveSnap(bool enabled);
    bool isPrimitiveSnapEnabled() const;

    void setGridStep(int step);
    int getGridStep() const;

    //расчет динамического шага сетки (для отрисовки во Viewport)
    double calculateDynamicGridStep(double zoomFactor) const;

private:
    //приватный конструктор (Singleton)
    explicit ObjectBindingManager(QObject* parent = nullptr);
    ObjectBindingManager(const ObjectBindingManager&) = delete;
    ObjectBindingManager& operator=(const ObjectBindingManager&) = delete;

    //внутренние методы расчета
    QPointF snapToGrid(const QPointF& pos, double zoomFactor) const;
    QPointF snapToPrimitives(const QPointF& pos, const Scene* scene, double zoomFactor, BasePrimitive* excludePrimitive) const;

    bool m_gridSnapEnabled = true; //флаг активации привязки к сетке
    bool m_primitiveSnapEnabled = true; //флаг активации привязки к примитивам
    int m_gridStep = 50; //базовый шаг сетки
};
