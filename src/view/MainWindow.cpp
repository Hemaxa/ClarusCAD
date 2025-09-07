#include "MainWindow.h"
#include "ViewportWidget.h"
#include "Scene.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("ClarusCAD");
    resize(1280, 720);

    m_scene = new Scene();
    m_viewportWidget = new ViewportWidget(this);
    m_viewportWidget->setScene(m_scene);
    setCentralWidget(m_viewportWidget);

    m_scene->addPoint(Point(100, 150));
    m_scene->addPoint(Point(400, 300));
}

MainWindow::~MainWindow() {
    delete m_scene;
}
