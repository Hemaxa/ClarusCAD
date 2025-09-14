//ViewportPanelWidget - панель окна просмотра сцены

#pragma once

#include "BasePanelWidget.h"
#include "BasePrimitive.h"

class Scene;
class BaseCreationTool;
class BaseDrawingTool;

//наслдедуется от базового класса BasePanelWidget
class ViewportPanelWidget : public BasePanelWidget
{
    Q_OBJECT

public:
    //конструктор
    explicit ViewportPanelWidget(const QString& title, QWidget* parent = nullptr);

    //этими методами MainWindow настраивает и управляет вьюпортом
    void setScene(Scene* scene); //метод установки сцены для отрисовки
    void setActiveTool(BaseCreationTool* tool); //метод установки активного элемента
    void setDrawingTools(const std::map<PrimitiveType, std::unique_ptr<BaseDrawingTool>>* tools); //метод установки отрисовщиков
    void update(); //метод перерисовки сцены

private:
    Scene* m_scene = nullptr; //указатель на сцену
    BaseCreationTool* m_activeTool = nullptr; //указатель на выбранный инструмент
    const std::map<PrimitiveType, std::unique_ptr<BaseDrawingTool>>* m_drawingTools = nullptr; //указатель на мапу отрисовщиков

    void paintCanvas(QPaintEvent* event); //метод отрисовки на холсте
    bool eventFilter(QObject* obj, QEvent* event); //метод перехвата действий с холстом
};
