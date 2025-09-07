#pragma once

#include <QMainWindow>
#include "Point.h"

class ViewportWidget;
class Scene;
class QListWidget;
class LinePropertiesWidget;
class BaseTool;
class ToolbarPanel;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void handleCreateSegment(const Point& start, const Point& end);
    void activateLineCreationTool();

private:
    void createDockWindows();

    Scene* m_scene;
    ViewportWidget* m_viewportWidget;
    QListWidget* m_sceneObjectsList;
    LinePropertiesWidget* m_propertiesWidget;
    ToolbarPanel* m_toolbarPanel; // Новая панель инструментов

    // Инструменты
    BaseTool* m_currentTool;
    BaseTool* m_lineCreationTool;
};
