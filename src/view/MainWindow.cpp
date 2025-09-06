#include "MainWindow.h"
#include "ViewportWidget.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("ClarusCAD");
    resize(1280, 720);

    m_viewportWidget = new ViewportWidget(this);
    setCentralWidget(m_viewportWidget);
}

MainWindow::~MainWindow() {}
