//NavigationPanelWidget - панель навигации по сцене

#pragma once

#include "BasePanelWidget.h"

class QToolButton;

//наслдедуется от базового класса BasePanelWidget
class NavigationPanelWidget : public BasePanelWidget
{
    Q_OBJECT

public:
    /**
     * @brief Конструктор панели навигации.
     */
    explicit NavigationPanelWidget(const QString& title, QWidget* parent = nullptr);

signals:
    // Сигналы для MainWindow
    
    /**
     * @brief Сигнал приближения.
     */
    void zoomInClicked();

    /**
     * @brief Сигнал отдаления.
     */
    void zoomOutClicked();

    /**
     * @brief Сигнал масштабирования по границам (Zoom Extents).
     */
    void zoomExtentsClicked();

    /**
     * @brief Сигнал поворота по часовой стрелке.
     */
    void rotateCWClicked();

    /**
     * @brief Сигнал поворота против часовой стрелки.
     */
    void rotateCCWClicked();

private:
    // Указатели на кпопки управления
    QToolButton* m_zoomInBtn;
    QToolButton* m_zoomOutBtn;
    QToolButton* m_zoomExtentsBtn;
    QToolButton* m_rotateCWBtn;
    QToolButton* m_rotateCCWBtn;
};
