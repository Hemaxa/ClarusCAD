#include "MainWindow.h"
#include "Scene.h"
#include "BaseDrawingTool.h"
#include "DeleteTool.h"
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
#include <QKeyEvent>
#include <QCursor>

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
    m_deleteTool = new DeleteTool(this);
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
    connect(m_toolbarPanel, &ToolbarPanelWidget::deleteToolActivated, this, &MainWindow::activateDeleteTool);
    connect(m_toolbarPanel, &ToolbarPanelWidget::segmentToolActivated, this, &MainWindow::activateSegmentCreationTool);

    //3) главное окно сообщает панели объектов, что сцена изменилась -> панель объектов обновляется
    connect(this, &MainWindow::sceneChanged, m_sceneObjectsPanel, &SceneObjectsPanelWidget::updateView);

    //4) главное окно сообщает панели свойств, что активирован инструмент -> панель свойств показывает пустую форму
    //QOverload используется, т.к. showPropertiesFor перегружен
    connect(this, &MainWindow::toolActivated, m_propertiesPanel, QOverload<PrimitiveType>::of(&PropertiesPanelWidget::showPropertiesFor));

    //5) главное окно сообщает панели свойств, что выбран объект -> сохраняется указатель на объект и панель свойств показывает его свойства
    connect(this, &MainWindow::objectSelected, this, [this](BasePrimitive* primitive) { m_selectedPrimitive = primitive; m_propertiesPanel->showPropertiesFor(primitive); });

    //6) панель объектов сцены сообщает, что пользователь выбрал примитив -> главное окно транслируем этот сигнал дальше (5 пункт)
    connect(m_sceneObjectsPanel, &SceneObjectsPanelWidget::primitiveSelected, this, &MainWindow::objectSelected);
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

void MainWindow::updateApplicationIcons()
{
    //отправка команды на обновление всем панелям, где есть иконки
    m_toolbarPanel->updateIcons();
}

void MainWindow::createActions()
{
    m_settingsAction = new QAction("Настройки...", this);
    m_settingsAction->setShortcut(QKeySequence::Preferences);
    connect(m_settingsAction, &QAction::triggered, this, &MainWindow::openSettingsDialog);
}

void MainWindow::createMenus()
{
    QMenu* fileMenu = menuBar()->addMenu("Файл");
    fileMenu->addAction(m_settingsAction);
}

void MainWindow::activateDeleteTool()
{
    deactivateCurrentTool(); //дективация старого инструмента

    m_currentTool = m_deleteTool; //обновляется указатель на текущий инструмент
    m_viewportPanel->setActiveTool(m_currentTool); //окну просмотра передается информация о выбранном инструменте
}

void MainWindow::activateSegmentCreationTool()
{
    deactivateCurrentTool(); //дективация старого инструмента

    m_currentTool = m_segmentCreationTool; //обновляется указатель на текущий инструмент
    m_viewportPanel->setActiveTool(m_currentTool); //окну просмотра передается информация о выбранном инструменте

    emit toolActivated(PrimitiveType::Segment); //посылается сигнал о выборе инструмента
}

void MainWindow::createNewSegment(const PointPrimitive& start, const PointPrimitive& end)
{
    auto* segment = new SegmentPrimitive(start, end);
    addPrimitiveToScene(segment); //метод добавления примитива в сцену
}

void MainWindow::openSettingsDialog()
{
    SettingsDialog dialog(this);

    //передаются текущие настройки приложения
    dialog.setCurrentTheme(ThemeManager::instance().currentThemeName());
    dialog.setGridStep(m_viewportPanel->getGridStep());

    //если нажата клавиша "ок", используются новые значения из диалога
    if (dialog.exec() == QDialog::Accepted) {
        QString selectedTheme = dialog.selectedThemeName();
        ThemeManager::instance().applyTheme(selectedTheme);

        //применяется новый шаг сетки
        int newGridStep = dialog.gridStep();
        m_viewportPanel->setGridStep(newGridStep);

        //обновляются UI элементы
        updateApplicationIcons();
    }
}

void MainWindow::deactivateCurrentTool()
{
    //если какой-либо инструмент активен
    if (m_currentTool) {
        m_currentTool->reset(); //сбрасывается состояние инструмента
        m_currentTool = nullptr; //сбрасывается указатель на инструмент
        m_viewportPanel->setActiveTool(nullptr); //сброс активного инструмента в окне просмотра
        m_toolbarPanel->clearSelection(); //кнопка на панели инструментов "отжимается"
        m_propertiesPanel->showPropertiesFor(nullptr); //панель свойств очищается
        m_viewportPanel->update(); //обновление окна просмотра
    }
}

void MainWindow::keyPressEvent(QKeyEvent* event)
{
    //1) Esc
    if (event->key() == Qt::Key_Escape) {
        deactivateCurrentTool();
        return;
    }

    //2) Delete / Backspace
    if ((event->key() == Qt::Key_Delete || event->key() == Qt::Key_Backspace) && m_selectedPrimitive) {
        m_scene->removePrimitive(m_selectedPrimitive);
        m_selectedPrimitive = nullptr;

        deactivateCurrentTool(); // Сбрасываем любой активный инструмент
        m_viewportPanel->update(); // Обновляем вид
        emit sceneChanged(m_scene); // Обновляем список объектов
        return; // Событие обработано, выходим
    }

    //3) Cmd + / Cmd -
    bool cursorIsOverViewport = m_viewportPanel->underMouse();

    if (cursorIsOverViewport) {
        // cmd + или cmd =
        if ((event->key() == Qt::Key_Plus || event->key() == Qt::Key_Equal) && (event->modifiers() & Qt::MetaModifier)) {
            //считается позиция курсора относительно холста окна просмотра
            QPoint mousePos = m_viewportPanel->getCanvas()->mapFromGlobal(QCursor::pos());
            m_viewportPanel->applyZoom(1.25, mousePos); //больше на 25%
            return;
        }
        // cmd -
        if (event->key() == Qt::Key_Minus && (event->modifiers() & Qt::MetaModifier)) {
            QPoint mousePos = m_viewportPanel->getCanvas()->mapFromGlobal(QCursor::pos());
            m_viewportPanel->applyZoom(0.75, mousePos); //меньше на 25%
            return;
        }
    }
    QMainWindow::keyPressEvent(event);
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
