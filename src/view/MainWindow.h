#pragma once

#include <QMainWindow>

#include "Point.h"

class ViewportWidget;
class Scene;
class QListWidget;
class LinePropertiesWidget;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void handleCreateSegment(const Point& start, const Point& end);
    void showLineCreationTool();

private:
    void createDockWindows();

    Scene* m_scene;
    ViewportWidget* m_viewportWidget;
    QListWidget* m_sceneObjectsList;
    LinePropertiesWidget* m_propertiesWidget;
    QWidget* m_bottomRightWidget;
};
