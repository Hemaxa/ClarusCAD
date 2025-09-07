#include "MainWindow.h"
#include "ViewportWidget.h"
#include "Scene.h"
#include "LinePropertiesWidget.h"
#include "ToolbarPanel.h" // Подключаем новую панель
#include "LineCreationTool.h" // Подключаем инструмент

#include <QDockWidget>
#include <QListWidget>
#include <QToolBar>
#include <QAction>
#include <QButtonGroup>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), m_currentTool(nullptr)
{
    setWindowTitle("ClarusCAD");
    resize(1280, 720);
    setDockNestingEnabled(true);

    m_scene = new Scene();

    // Создаем виджеты
    m_viewportWidget = new ViewportWidget();
    m_viewportWidget->setScene(m_scene);
    m_sceneObjectsList = new QListWidget();
    m_propertiesWidget = new LinePropertiesWidget();
    m_toolbarPanel = new ToolbarPanel(); // Создаем панель

    // Создаем док-виджеты
    createDockWindows();

    // Создаем инструменты
    m_lineCreationTool = new LineCreationTool(this);

    // Активируем инструмент по умолчанию
    activateLineCreationTool();

    // Соединяем сигналы и слоты
    connect(m_propertiesWidget, &LinePropertiesWidget::createSegmentRequested, this, &MainWindow::handleCreateSegment);
    connect(m_toolbarPanel, &ToolbarPanel::createLineToolActivated, this, &MainWindow::activateLineCreationTool);
}

MainWindow::~MainWindow() {
    delete m_scene;
}

void MainWindow::createDockWindows()
{
    setCentralWidget(nullptr);

    QDockWidget *viewportDock = new QDockWidget("Рабочая область", this);
    viewportDock->setWidget(m_viewportWidget);
    addDockWidget(Qt::LeftDockWidgetArea, viewportDock);

    QDockWidget *sceneListDock = new QDockWidget("Объекты на сцене", this);
    sceneListDock->setWidget(m_sceneObjectsList);
    addDockWidget(Qt::RightDockWidgetArea, sceneListDock);

    QDockWidget *propertiesDock = new QDockWidget("Параметры объекта", this);
    propertiesDock->setWidget(m_propertiesWidget);
    addDockWidget(Qt::LeftDockWidgetArea, propertiesDock);

    // Заменяем правый нижний виджет на панель инструментов
    QDockWidget *toolbarDock = new QDockWidget("Инструменты", this);
    toolbarDock->setWidget(m_toolbarPanel);
    addDockWidget(Qt::RightDockWidgetArea, toolbarDock);

    splitDockWidget(viewportDock, propertiesDock, Qt::Vertical);
    splitDockWidget(sceneListDock, toolbarDock, Qt::Vertical);
}

void MainWindow::handleCreateSegment(const Point& start, const Point& end)
{
    Segment segment(start, end);
    m_scene->addSegment(segment);

    m_sceneObjectsList->addItem(QString("Отрезок (%1, %2) - (%3, %4)")
                                    .arg(start.x()).arg(start.y()).arg(end.x()).arg(end.y()));
    m_viewportWidget->update();
}

void MainWindow::activateLineCreationTool()
{
    m_currentTool = m_lineCreationTool;
    m_viewportWidget->setActiveTool(m_currentTool);
    // Показываем виджет свойств для линии, если он был скрыт
    m_propertiesWidget->parentWidget()->show();
}
