#include "MainWindow.h"
#include "Scene.h"
#include "SegmentCreationTool.h"

// ГЛАВНОЕ ИСПРАВЛЕНИЕ: Убедитесь, что все эти #include на месте.
// Компилятор должен "видеть" чертежи всех классов, которые вы используете.
#include "ViewportPanelWidget.h"
#include "ToolbarPanelWidget.h"
#include "PropertiesPanelWidget.h"
#include "SceneObjectsPanelWidget.h"

#include <QDockWidget>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent), m_currentTool(nullptr)
{
    setWindowTitle("ClarusCAD");
    resize(1280, 720);
    setDockNestingEnabled(true);

    m_scene = new Scene();

    // ПЕРЕНЕСЕНО: Лучше сначала создать инструменты, потом окна, потом соединения
    createTools();
    createDockWindows();
    createConnections();

    activateSegmentCreationTool();
}

MainWindow::~MainWindow()
{
    delete m_scene;
}

// ДОБАВЛЕНО: Отдельный метод для создания инструментов
void MainWindow::createTools()
{
    m_segmentCreationTool = new SegmentCreationTool(this);
}

void MainWindow::createDockWindows()
{
    // Viewport
    m_viewportPanelWidget = new ViewportPanelWidget();
    m_viewportPanelWidget->setScene(m_scene);
    setCentralWidget(m_viewportPanelWidget);

    // Панели
    m_toolbarPanel = new ToolbarPanelWidget("Инструменты", this);
    m_propertiesPanel = new PropertiesPanelWidget("Свойства", this);
    m_sceneObjectsPanel = new SceneObjectsPanelWidget("Объекты на сцене", this);

    addDockWidget(Qt::LeftDockWidgetArea, m_toolbarPanel);
    addDockWidget(Qt::LeftDockWidgetArea, m_propertiesPanel);
    addDockWidget(Qt::RightDockWidgetArea, m_sceneObjectsPanel);
}

// ДОБАВЛЕНО: Отдельный метод для всех соединений
void MainWindow::createConnections()
{
    // Инструмент "Отрезок" сообщает, что создал примитив
    connect(m_segmentCreationTool, &SegmentCreationTool::primitiveCreated, this, &MainWindow::addPrimitiveToScene);

    // Кнопка на панели инструментов сообщает, что нужно активировать инструмент
    connect(m_toolbarPanel, &ToolbarPanelWidget::segmentToolActivated, this, &MainWindow::activateSegmentCreationTool);

    // Панель свойств сообщает, что нужно создать отрезок по координатам
    connect(m_propertiesPanel, &PropertiesPanelWidget::createSegmentRequested, this, &MainWindow::handleCreateSegmentFromProperties);

    // Главное окно сообщает панели объектов, что сцена изменилась и нужно обновиться
    connect(this, &MainWindow::sceneChanged, m_sceneObjectsPanel, &SceneObjectsPanelWidget::updateView);
}

void MainWindow::activateSegmentCreationTool()
{
    m_currentTool = m_segmentCreationTool;
    m_viewportPanelWidget->setActiveTool(m_currentTool);
}

void MainWindow::addPrimitiveToScene(BasePrimitive* primitive)
{
    if (primitive) {
        m_scene->addPrimitive(std::unique_ptr<BasePrimitive>(primitive));
        m_viewportPanelWidget->update();

        // ИСПРАВЛЕНИЕ: Нужно отправлять сигнал, чтобы список объектов обновился
        emit sceneChanged(m_scene);

        emit objectSelected(primitive);
    }
}

void MainWindow::handleCreateSegmentFromProperties(const PointCreationPrimitive& start, const PointCreationPrimitive& end)
{
    auto segment = std::make_unique<SegmentCreationPrimitive>(start, end);
    addPrimitiveToScene(segment.release());
}
