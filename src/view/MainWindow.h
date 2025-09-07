#pragma once

#include <QMainWindow>

class ViewportWidget;
class Scene;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    ViewportWidget* m_viewportWidget;
    Scene* m_scene;
};
