//MoveTool - инструмент перемещения по сцене

#pragma once

#include "BaseCreationTool.h"

#include <QTimer>
#include <QPoint>

class ViewportPanelWidget;

class MoveTool : public BaseCreationTool
{
    Q_OBJECT

public:
    //конструктор
    explicit MoveTool(QObject* parent = nullptr);

    //переопределение методов действий мыши
    void onMousePress(QMouseEvent* event, Scene* scene, ViewportPanelWidget* viewport) override;
    void onMouseMove(QMouseEvent* event, Scene* scene, ViewportPanelWidget* viewport) override;
    void onMouseRelease(QMouseEvent* event, Scene* scene, ViewportPanelWidget* viewport) override;

    //переопределение метода очистки инструмента
    void reset() override;

    //метод активации активации инструмента
    void activate(ViewportPanelWidget* viewport);

    //метод деактивации инструмента
    void deactivate();

    //геттер для проверки состояния
    bool isActive() const;

public slots:
    //слот, получающий новую позицию мыши от ViewportPanelWidget
    void updateMousePosition(const QPoint& screenPos);

private slots:
    //слот, понарамирующий сцену
    void onPanTimerTimeout();

private:
    QTimer* m_panTimer; //таймер для запуска перемещения
    ViewportPanelWidget* m_viewport = nullptr; //указатель на окно просмотра
    QPoint m_currentMousePos; //позиция мыши
    bool m_isActive = false; //флаг включения/выключения инструмента

    const int m_borderThreshold = 300; //отступ от края экрана, в области которого инструмент активен
    const double m_maxPanSpeed = 20.0; //максимальная скорость перемещения
};
