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
    /**
     * @brief Получить доступ к синглтону.
     */
    static ObjectBindingManager& instance();

    /**
     * @brief Получить скорректированную точку с учетом привязок.
     * @param currentPos Текущая позиция курсора.
     * @param scene Сцена.
     * @param zoomFactor Коэффициент зума.
     * @param excludePrimitive Примитив, который нужно игнорировать (например, перемещаемый).
     * @return Скорректированная точка (прилипшая к сетке или объекту).
     */
    QPointF getSnappedPoint(const QPointF& currentPos, const Scene* scene, double zoomFactor, BasePrimitive* excludePrimitive = nullptr) const;

    // Методы управления состоянием привязок
    
    /**
     * @brief Включить/выключить привязку к сетке.
     */
    void setGridSnap(bool enabled);
    
    bool isGridSnapEnabled() const;

    /**
     * @brief Включить/выключить привязку к примитивам.
     */
    void setPrimitiveSnap(bool enabled);
    
    bool isPrimitiveSnapEnabled() const;

    /**
     * @brief Установить шаг сетки.
     */
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
