//NavigationPanelWidget - панель навигации по сцене

#pragma once

#include "BasePanelWidget.h"

class QToolButton;

//наслдедуется от базового класса BasePanelWidget
class NavigationPanelWidget : public BasePanelWidget
{
    Q_OBJECT

public:
    //конструктор
    explicit NavigationPanelWidget(const QString& title, QWidget* parent = nullptr);

signals:
    //Сигналы для MainWindow
    void zoomInClicked();
    void zoomOutClicked();
    void zoomExtentsClicked();
    void rotateCWClicked();
    void rotateCCWClicked();

private:
    //указатели на соответствующие кнопки
    QToolButton* m_zoomInBtn;
    QToolButton* m_zoomOutBtn;
    QToolButton* m_zoomExtentsBtn;
    QToolButton* m_rotateCWBtn;
    QToolButton* m_rotateCCWBtn;
};
