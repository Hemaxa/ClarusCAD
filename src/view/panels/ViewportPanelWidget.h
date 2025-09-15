//ViewportPanelWidget - панель окна просмотра сцены

#pragma once

#include "BasePanelWidget.h"
#include "BasePrimitive.h"

#include <QPointF>

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
    void setGridStep(int step); //метод установки шага сетки

    int getGridStep() const; //геттер для шага сетки
    QWidget* getCanvas() const; //геттер для холста
    void update(); //метод перерисовки сцены

    //методы для трансформации координат
    QPointF worldToScreen(const QPointF& worldPos) const;
    QPointF screenToWorld(const QPointF& screenPos) const;

public slots:
    void applyZoom(double factor, const QPoint& anchorPoint); //метод применения масштабирования

private:
    Scene* m_scene = nullptr; //указатель на сцену
    BaseCreationTool* m_activeTool = nullptr; //указатель на выбранный инструмент
    const std::map<PrimitiveType, std::unique_ptr<BaseDrawingTool>>* m_drawingTools = nullptr; //указатель на мапу отрисовщиков

    int m_gridStep = 50; //шаг сетки по умолчанию
    QPointF m_panOffset{0.0, 0.0}; //смещение вида (панорамирование), хранит данные о сдвиге сцены
    double m_zoomFactor = 1.0; //коэффициент масштабирования (1.0 = 100%)
    bool m_isPanning = false; //флаг активации перемещения по сцене
    QPoint m_lastPanPos; //последняя позиция курсора во время панорамирования для расчета смещения

    bool eventFilter(QObject* obj, QEvent* event) override; //метод перехвата действий с холстом
    void paintCanvas(QPaintEvent* event); //метод отрисовки на холсте
    void paintGrid(QPainter& painter); //метод отрисовки сетки
    void paintGizmo(QPainter& painter); //метод отрисовки гизмо
};
