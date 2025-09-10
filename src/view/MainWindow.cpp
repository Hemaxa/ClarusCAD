#include "MainWindow.h"
#include "Scene.h"
#include "SegmentCreationTool.h"
#include "ViewportPanelWidget.h"
#include "ToolbarPanelWidget.h"
#include "PropertiesPanelWidget.h"
#include "SceneObjectsPanelWidget.h"

#include <QDockWidget>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent), m_currentTool(nullptr)
{
    //настройки окна приложения
    setWindowTitle("ClarusCAD");
    setWindowState(Qt::WindowMaximized);
    setDockNestingEnabled(true);

    //создание экземпляра сцены
    m_scene = new Scene();

    //вызов методов создания интерфейса и связей
    createTools();
    createDockWindows();
    createConnections();
}

MainWindow::~MainWindow()
{
    delete m_scene;
}

void MainWindow::createTools()
{
    m_segmentCreationTool = new SegmentCreationTool(this);
}

void MainWindow::createDockWindows()
{
    //создание интерфейсных панелей
    m_viewportPanel = new ViewportPanelWidget("Рабочая область", this);
    m_toolbarPanel = new ToolbarPanelWidget("Инструменты", this);
    m_propertiesPanel = new PropertiesPanelWidget("Свойства", this);
    m_sceneObjectsPanel = new SceneObjectsPanelWidget("Список объектов", this);

    //передача сцены в панель просмотра
    m_viewportPanel->setScene(m_scene);

    //расстановка панелей интерфейса
    //левая колонка
    addDockWidget(Qt::LeftDockWidgetArea, m_toolbarPanel);
    addDockWidget(Qt::LeftDockWidgetArea, m_propertiesPanel);
    splitDockWidget(m_toolbarPanel, m_propertiesPanel, Qt::Vertical);

    //правая колонка
    addDockWidget(Qt::RightDockWidgetArea, m_sceneObjectsPanel);

    //встраивание рабочей области между левой и правой колонками
    splitDockWidget(m_toolbarPanel, m_viewportPanel, Qt::Horizontal);
    splitDockWidget(m_viewportPanel, m_sceneObjectsPanel, Qt::Horizontal);
}

void MainWindow::createConnections()
{
    //инструмент "Отрезок" сообщает, что создал примитив
    connect(m_segmentCreationTool, &SegmentCreationTool::primitiveCreated, this, &MainWindow::addPrimitiveToScene);

    //кнопка на панели инструментов сообщает, что нужно активировать инструмент
    connect(m_toolbarPanel, &ToolbarPanelWidget::segmentToolActivated, this, &MainWindow::activateSegmentCreationTool);

    //панель свойств сообщает, что нужно создать отрезок по координатам
    connect(m_propertiesPanel, &PropertiesPanelWidget::createSegmentRequested, this, &MainWindow::handleCreateSegmentFromProperties);

    //главное окно сообщает панели объектов, что сцена изменилась и нужно обновиться
    connect(this, &MainWindow::sceneChanged, m_sceneObjectsPanel, &SceneObjectsPanelWidget::updateView);

    //главное окно сообщает панели объектов, что нужно показать параметры выбранного объекта
    connect(this, &MainWindow::toolActivated, m_propertiesPanel, QOverload<PrimitiveType>::of(&PropertiesPanelWidget::showPropertiesFor));

    connect(this, &MainWindow::objectSelected, m_propertiesPanel, QOverload<BasePrimitive*>::of(&PropertiesPanelWidget::showPropertiesFor));

    connect(m_sceneObjectsPanel, &SceneObjectsPanelWidget::primitiveSelected, this, &MainWindow::objectSelected);
}

void MainWindow::activateSegmentCreationTool()
{
    m_currentTool = m_segmentCreationTool;
    m_viewportPanel->setActiveTool(m_currentTool);

    emit toolActivated(PrimitiveType::Segment);
}

void MainWindow::addPrimitiveToScene(BasePrimitive* primitive)
{
    if (primitive) {
        m_scene->addPrimitive(std::unique_ptr<BasePrimitive>(primitive));
        m_viewportPanel->update();

        emit sceneChanged(m_scene);
        emit objectSelected(primitive);
    }
}

void MainWindow::handleCreateSegmentFromProperties(const PointCreationPrimitive& start, const PointCreationPrimitive& end)
{
    auto segment = std::make_unique<SegmentCreationPrimitive>(start, end);
    addPrimitiveToScene(segment.release());
}

void MainWindow::showEvent(QShowEvent* event)
{
    QMainWindow::showEvent(event);

    if (!m_isInitialResizeDone) {
        m_isInitialResizeDone = true;

        //определение размеров по ширине (процент от ширины экрана)
        const double leftColumnPercentage = 0.20;
        const double rightColumnPercentage = 0.20;

        //определение размеров колонок
        int totalWidth = width();
        int leftWidth = static_cast<int>(totalWidth * leftColumnPercentage);
        int rightWidth = static_cast<int>(totalWidth * rightColumnPercentage);
        int middleWidth = totalWidth - leftWidth - rightWidth;

        resizeDocks({m_toolbarPanel, m_viewportPanel, m_sceneObjectsPanel}, {leftWidth, middleWidth, rightWidth}, Qt::Horizontal);

        //определение размеров по высоте (процент от высоты экрана)
        const double toolbarHeightPercentage = 0.70;
        const double propertiesHeightPercentage = 0.30;

        int totalHeight = height();
        int toolbarHeight = static_cast<int>(totalHeight * toolbarHeightPercentage);
        int propertiesHeight = static_cast<int>(totalHeight * propertiesHeightPercentage);

        resizeDocks({m_toolbarPanel, m_propertiesPanel}, {toolbarHeight, propertiesHeight}, Qt::Vertical);
    }
}
