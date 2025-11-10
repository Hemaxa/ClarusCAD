//ViewportPanelWidget - панель окна просмотра сцены

#pragma once

#include "BasePanelWidget.h"
#include "BasePrimitive.h"
#include "EnumManager.h"

#include <QPointF>

class Scene;
class ViewportCamera;
class ThemeManager;
class BaseCreationTool;
class BaseDrawingTool;
class QLabel;

//наслдедуется от базового класса BasePanelWidget
class ViewportPanelWidget : public BasePanelWidget
{
    Q_OBJECT

public:
    //конструктор и деструктор
    explicit ViewportPanelWidget(const QString& title, QWidget* parent = nullptr);
    ~ViewportPanelWidget();

    //этими методами MainWindow настраивает и управляет вьюпортом
    void setScene(Scene* scene); //метод установки сцены для отрисовки
    void setActiveTool(BaseCreationTool* tool); //метод установки активного элемента
    void setGridStep(int step); //метод установки шага сетки

    int getGridStep() const; //геттер для шага сетки
    double getDynamicGridStep() const; //геттер для масштабируемого шага сетки
    double getZoomFactor() const; //геттер для коэффициента масштабирования
    QWidget* getCanvas() const; //геттер для холста
    QPointF getSnappedPoint(const QPointF& worldPos) const; //геттер для точки привязки

    //методы для трансформации координат
    QPointF worldToScreen(const QPointF& worldPos) const;
    QPointF screenToWorld(const QPointF& screenPos) const;

    void update(); //метод перерисовки сцены

public slots:
    void applyZoom(double factor, const QPoint& anchorPoint); //слот применения масштабирования
    void rotateScene();

    //методы для зумирования
    void zoomIn(); //зум к центру
    void zoomIn(const QPoint& anchorPoint); //зум к точке
    void zoomOut(); //отдаление от центра
    void zoomOut(const QPoint& anchorPoint); //отдаление от точки
    void zoomToExtents();

    void setZoomStep(double step); //слот установки шага увеличения/уменьшения
    void setCoordinateSystem(CoordinateSystemType type); //слот установки системы координат
    void setGridSnapEnabled(bool enabled); //слот включения/выключения привязки к сетке
    void setPrimitiveSnapEnabled(bool enabled); //слот включения/выключения привязки к примитивам
    void setSelectedPrimitive(BasePrimitive* primitive); //слот выбранных объектов

    void panWorld(const QPointF& worldDelta);

signals:
    //cигнал, который передает позицию мыши
    void mouseMoved(const QPoint& screenPos);

private slots:
    // НОВЫЙ СЛОТ для связи с камерой
    void onCameraUpdated();

private:
    Scene* m_scene = nullptr; //указатель на сцену
    BasePrimitive* m_selectedPrimitive = nullptr; //указатель на выбранные объекты
    QLabel* m_infoLabel; //указатель на info-панель
    BaseCreationTool* m_activeTool = nullptr; //указатель на выбранный инструмент
    std::map<PrimitiveType, std::unique_ptr<BaseDrawingTool>> m_drawingTools; //мапа отрисовщиков
    ThemeManager* m_themeManager = nullptr; //указатель на менеджер тем

    //параметры панели по умолчанию
    int m_gridStep = 50; //шаг сетки
    double m_zoomStep = 1.25; //шаг увеличения/уменьшения
    QPoint m_lastPanPos; //последняя позиция курсора во время панорамирования для расчета смещения

    bool m_isPanning = false; //флаг активации перемещения по сцене (зажатие ЛКМ)
    bool m_isGridSnapEnabled = true; //флаг активации привязки к сетке
    bool m_isPrimitiveSnapEnabled = true; //флаг активации привязки к примитивам

    QPointF m_currentMouseWorldPos; //текущая позиция мыши в мировых координатах
    double m_gridMultiplier = 1.0; //текущий множитель сетки
    CoordinateSystemType m_coordSystemType = CoordinateSystemType::Cartesian; //текущая система координат

    ViewportCamera* m_camera;

    QRect getGizmoRect() const; // Область нажатия для гизмо

    bool eventFilter(QObject* obj, QEvent* event) override; //метод перехвата действий с холстом
    double calculateDynamicGridStep() const; //метод расчета отмасштабированного шага сетки
    QPointF snapToGrid(const QPointF& worldPos) const; //метод привязки к сетке
    QPointF snapToPrimitives(const QPointF& worldPos) const; //метод привязки к примитивам
    void paintCanvas(QPaintEvent* event); //метод отрисовки на холсте
    void paintGrid(QPainter& painter, const QTransform& worldTransform); //метод отрисовки сетки
    void paintGizmo(QPainter& painter); //метод отрисовки гизмо
    void createDrawingTools(); //метод создание отрисовщиков
    void updateInfoLabel(); //метод обновления содержимого info-панели
};
