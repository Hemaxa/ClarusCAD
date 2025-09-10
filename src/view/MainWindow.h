#pragma once

#include "PointCreationPrimitive.h"

#include <QMainWindow>
#include <QShowEvent>

//предварительные объявления всех классов
class Scene;
class BasePrimitive;
class BaseTool;
class SegmentCreationTool;
class ViewportPanelWidget;
class ToolbarPanelWidget;
class PropertiesPanelWidget;
class SceneObjectsPanelWidget;

//QMainWindow - базовый шаблон Qt для создания главного окна
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    //конструктор и деструктор
    MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

public slots:
    //слот создания отрезка
    void handleCreateSegmentFromProperties(const PointCreationPrimitive& start, const PointCreationPrimitive& end);

private slots:
    //слот для активации инструмента создания отрезка
    void activateSegmentCreationTool();

    //слот добавления примитива в сцену
    void addPrimitiveToScene(BasePrimitive* primitive);

signals:
    //сигнал, который сообщает панелям, что сцена изменилась
    void sceneChanged(const Scene* scene);

    //сигнал, который сообщает, что объект выбран
    void objectSelected(BasePrimitive* primitive);

    //сигнал, который сообщает, что инструмент был активирован
    void toolActivated(PrimitiveType type);

private:
    void createTools(); //метод создания инструментов
    void createDockWindows(); //метод создания интерфейсных панелей
    void createConnections(); //метод создания взаимодействий
    void showEvent(QShowEvent* event) override; //переопределение метода создания первичного окна приложения

    Scene* m_scene; //указатель на объект сцены
    BaseTool* m_currentTool; //указатель на выбранный инструмент

    bool m_isInitialResizeDone = false;

    //инструменты
    SegmentCreationTool* m_segmentCreationTool;

    //интерфейсные панели
    ViewportPanelWidget* m_viewportPanel;
    ToolbarPanelWidget* m_toolbarPanel;
    PropertiesPanelWidget* m_propertiesPanel;
    SceneObjectsPanelWidget* m_sceneObjectsPanel;
};
