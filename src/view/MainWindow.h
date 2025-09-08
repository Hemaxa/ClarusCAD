#pragma once
#include <QMainWindow>
#include <memory>
#include "PointCreationPrimitive.h"

// Предварительные объявления всех классов
class Scene;
class BaseTool;
class ViewportPanelWidget;
class SegmentCreationTool;
class BasePrimitive;
class ToolbarPanelWidget;
class PropertiesPanelWidget;
class SceneObjectsPanelWidget;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

public slots:
    // Слот для создания отрезка из панели свойств
    void handleCreateSegmentFromProperties(const PointCreationPrimitive& start, const PointCreationPrimitive& end);

private slots:
    // Слот для активации инструмента
    void activateSegmentCreationTool();
    // Слот для добавления примитива в сцену (из любого источника)
    void addPrimitiveToScene(BasePrimitive* primitive);

signals:
    // Сигнал, который сообщает панелям, что сцена изменилась
    void sceneChanged(const Scene* scene);
    // Сигнал для будущего: сообщает, что объект выбран
    void objectSelected(BasePrimitive* primitive);

private:
    void createTools();
    void createDockWindows();
    void createConnections();

    Scene* m_scene;
    BaseTool* m_currentTool;

    // Инструменты
    SegmentCreationTool* m_segmentCreationTool;

    // Панели
    ViewportPanelWidget* m_viewportPanelWidget;
    ToolbarPanelWidget* m_toolbarPanel;
    PropertiesPanelWidget* m_propertiesPanel;
    SceneObjectsPanelWidget* m_sceneObjectsPanel;
};
