#include "MainWindow.h"
#include "Scene.h"
#include "BaseDrawingTool.h"
#include "SegmentPrimitive.h"
#include "SegmentDrawingTool.h"
#include "SegmentCreationTool.h"
#include "ViewportPanelWidget.h"
#include "ToolbarPanelWidget.h"
#include "PropertiesPanelWidget.h"
#include "SceneObjectsPanelWidget.h"
#include "SettingsDialog.h"
#include "ThemeManager.h"

#include <QDockWidget>
#include <QMenuBar>
#include <QMenu>

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent), m_currentTool(nullptr)
{
    //настройки окна приложения
    setWindowTitle("ClarusCAD");
    setWindowState(Qt::WindowMaximized);
    setDockNestingEnabled(true);

    //применение сохраненной темы при запуске
    ThemeManager::instance().reloadCurrentTheme();

    //создание экземпляра сцены
    m_scene = new Scene();

    //вызов всех методов создания
    createTools();
    createDrawingTools();
    createPanelWindows();
    createConnections();
    createActions();
    createMenus();

    m_propertiesPanel->showPropertiesFor(nullptr);
}

MainWindow::~MainWindow()
{
    delete m_scene;
}

void MainWindow::createTools()
{
    m_segmentCreationTool = new SegmentCreationTool(this);
}

void MainWindow::createDrawingTools()
{
    m_drawingTools[PrimitiveType::Segment] = std::make_unique<SegmentDrawingTool>();
}

void MainWindow::createPanelWindows()
{
    //создание интерфейсных панелей
    m_viewportPanel = new ViewportPanelWidget("Рабочая область", this);
    m_toolbarPanel = new ToolbarPanelWidget("Инструменты", this);
    m_propertiesPanel = new PropertiesPanelWidget("Свойства", this);
    m_sceneObjectsPanel = new SceneObjectsPanelWidget("Список объектов", this);

    //установка оформления из файлов тем
    m_viewportPanel->setProperty("class", "ViewportPanel");
    m_sceneObjectsPanel->setProperty("class", "SceneObjectsPanel");

    //передача сцены и отрисовщиков в панель просмотра
    m_viewportPanel->setScene(m_scene);
    m_viewportPanel->setDrawingTools(&m_drawingTools);

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
    //1) инструмент и панель свойств сообщают данные -> в главном окне вызывается слот создания соответствующего объекта
    connect(m_segmentCreationTool, &SegmentCreationTool::segmentDataReady, this, &MainWindow::createNewSegment);
    connect(m_propertiesPanel, &PropertiesPanelWidget::createSegmentRequested, this, &MainWindow::createNewSegment);

    //2) панель инструментов сообщает о нажатии кнопки -> в главном окне активируется соответствующий инструмент
    connect(m_toolbarPanel, &ToolbarPanelWidget::segmentToolActivated, this, &MainWindow::activateSegmentCreationTool);

    //3) главное окно сообщает панели объектов, что сцена изменилась -> панель объектов обновляется
    connect(this, &MainWindow::sceneChanged, m_sceneObjectsPanel, &SceneObjectsPanelWidget::updateView);

    //4) главное окно сообщает панели свойств, что активирован инструмент -> панель свойств показывает пустую форму
    //QOverload используется, т.к. showPropertiesFor перегружен
    connect(this, &MainWindow::toolActivated, m_propertiesPanel, QOverload<PrimitiveType>::of(&PropertiesPanelWidget::showPropertiesFor));

    //5) главное окно сообщает панели свойств, что выбран объект -> панель свойств показывает его свойства
    connect(this, &MainWindow::objectSelected, m_propertiesPanel, QOverload<BasePrimitive*>::of(&PropertiesPanelWidget::showPropertiesFor));

    //6) панель объектов сцены сообщает, что пользователь выбрал примитив -> главное окно транслируем этот сигнал дальше (5 пункт)
    connect(m_sceneObjectsPanel, &SceneObjectsPanelWidget::primitiveSelected, this, &MainWindow::objectSelected);
}

void MainWindow::activateSegmentCreationTool()
{
    m_currentTool = m_segmentCreationTool; //обновляется указатель на текущий инструмент
    m_viewportPanel->setActiveTool(m_currentTool); //окну просмотра передается информация о выбранном инструменте

    emit toolActivated(PrimitiveType::Segment); //посылается сигнал о выборе инструмента
}

void MainWindow::createNewSegment(const PointPrimitive& start, const PointPrimitive& end)
{
    auto* segment = new SegmentPrimitive(start, end);
    addPrimitiveToScene(segment); //метод добавления примитива в сцену
}

void MainWindow::addPrimitiveToScene(BasePrimitive* primitive)
{
    if (primitive) {
        m_scene->addPrimitive(std::unique_ptr<BasePrimitive>(primitive)); //добавление примитива в вектор сцены
        m_viewportPanel->update(); //обновление окна просмотра

        emit sceneChanged(m_scene); //посылается сигнал, что сцена изменилась
        emit objectSelected(primitive); //посылается сигнал, о выбранном объекте
    }
}

// НОВЫЙ МЕТОД
void MainWindow::createActions()
{
    m_settingsAction = new QAction("Настройки...", this);
    m_settingsAction->setShortcut(QKeySequence::Preferences);
    connect(m_settingsAction, &QAction::triggered, this, &MainWindow::openSettingsDialog);
}

// НОВЫЙ МЕТОД
void MainWindow::createMenus()
{
    QMenu* fileMenu = menuBar()->addMenu("Файл");
    fileMenu->addAction(m_settingsAction);
}

// НОВЫЙ СЛОТ
void MainWindow::openSettingsDialog()
{
    SettingsDialog dialog(this);

    // Передаем в диалог ТЕКУЩИЕ настройки приложения
    dialog.setCurrentTheme(ThemeManager::instance().currentThemeName());
    // Примечание: здесь также нужно будет передать текущий шаг сетки, когда вы его реализуете
    // dialog.setGridStep(m_viewportPanel->getGridStep());

    if (dialog.exec() == QDialog::Accepted) {
        // Если пользователь нажал "OK", получаем НОВЫЕ значения из диалога
        QString selectedTheme = dialog.selectedThemeName();
        ThemeManager::instance().applyTheme(selectedTheme);

        // Также получаем и применяем новый шаг сетки
        // int newGridStep = dialog.gridStep();
        // m_viewportPanel->setGridStep(newGridStep);

        // ВАЖНО: После применения темы обновить все UI элементы
        updateApplicationIcons();
    }
}

// НОВЫЙ МЕТОД
void MainWindow::updateApplicationIcons()
{
    // Отправляем команду на обновление всем панелям, где есть иконки
    m_toolbarPanel->updateIcons();
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
