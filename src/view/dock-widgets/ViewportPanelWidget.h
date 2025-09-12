//ViewportPanelWidget - панель окна просмотра сцены

#pragma once

#include "BaseDockWidget.h"

class Scene;
class BaseTool;

//наслдедуется от базового класса BaseDockWidget
class ViewportPanelWidget : public BaseDockWidget
{
    Q_OBJECT

public:
    //конструктор
    explicit ViewportPanelWidget(const QString& title, QWidget* parent = nullptr);

    void setScene(Scene* scene); //метод установки сцены для отрисовки
    void setActiveTool(BaseTool* tool); //метод установки активного элемента
    void update(); //метод перерисовки содержимого

private:
    Scene* m_scene = nullptr; //указатель на сцену
    BaseTool* m_activeTool = nullptr; //указатель на выбранный инструмент

    void paintCanvas(QPaintEvent* event); //метод отрисовки на холсте
    bool eventFilter(QObject* obj, QEvent* event); //метод перехвата действий с холстом
};
