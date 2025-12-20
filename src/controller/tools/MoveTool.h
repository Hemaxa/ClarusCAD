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
    /**
     * @brief Конструктор инструмента перемещения (панорамирования).
     */
    explicit MoveTool(QObject* parent = nullptr);

    //переопределение методов действий мыши
    void onMousePress(QMouseEvent* event, Scene* scene, ViewportPanelWidget* viewport) override;
    void onMouseMove(QMouseEvent* event, Scene* scene, ViewportPanelWidget* viewport) override;
    void onMouseRelease(QMouseEvent* event, Scene* scene, ViewportPanelWidget* viewport) override;

    //переопределение метода очистки инструмента
    void reset() override;

    /**
     * @brief Активировать инструмент перемещения.
     * @param viewport Виджет, к которому привязывается перемещение.
     */
    void activate(ViewportPanelWidget* viewport);

    /**
     * @brief Деактивировать инструмент.
     */
    void deactivate();

    /**
     * @brief Проверить, активен ли инструмент.
     */
    bool isActive() const;

public slots:
    /**
     * @brief Слот обновления позиции мыши (для авто-панорамирования у краев).
     * @param screenPos Позиция курсора на экране.
     */
    void updateMousePosition(const QPoint& screenPos);

private slots:
    /**
     * @brief Слот таймер для плавного панорамирования.
     */
    void onPanTimerTimeout();

private:
    QTimer* m_panTimer; //таймер для запуска перемещения
    ViewportPanelWidget* m_viewport = nullptr; //указатель на окно просмотра
    QPoint m_currentMousePos; //позиция мыши
    bool m_isActive = false; //флаг включения/выключения инструмента

    const int m_borderThreshold = 300; //отступ от края экрана, в области которого инструмент активен
    const double m_maxPanSpeed = 20.0; //максимальная скорость перемещения
};
