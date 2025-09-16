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
#include "SceneSettingsPanelWidget.h"
#include "ConsolePanelWidget.h"
#include "CommandParser.h"
#include "SettingsDialog.h"
#include "ThemeManager.h"

#include <QDockWidget>
#include <QMenuBar>
#include <QMenu>
#include <QKeyEvent>
#include <QCursor>
#include <QApplication>

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

    m_commandParser = new CommandParser(this);

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
    m_sceneSettingsPanel = new SceneSettingsPanelWidget("Параметры сцены", this);
    m_consolePanel = new ConsolePanelWidget("Консольный ввод", this);

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
    addDockWidget(Qt::LeftDockWidgetArea, m_consolePanel);
    splitDockWidget(m_toolbarPanel, m_propertiesPanel, Qt::Vertical);
    splitDockWidget(m_propertiesPanel, m_consolePanel, Qt::Vertical);

    //правая колонка
    addDockWidget(Qt::RightDockWidgetArea, m_sceneObjectsPanel);
    addDockWidget(Qt::RightDockWidgetArea, m_sceneSettingsPanel);
    splitDockWidget(m_sceneObjectsPanel, m_sceneSettingsPanel, Qt::Vertical);

    //верхняя строка
    splitDockWidget(m_toolbarPanel, m_viewportPanel, Qt::Horizontal);
    splitDockWidget(m_viewportPanel, m_sceneObjectsPanel, Qt::Horizontal);

    //нижняя строка
    splitDockWidget(m_consolePanel, m_propertiesPanel, Qt::Horizontal);
    splitDockWidget(m_propertiesPanel, m_sceneSettingsPanel, Qt::Horizontal);
}

void MainWindow::createConnections()
{
    //1) инструмент и панель свойств сообщают данные -> в главном окне вызывается слот создания соответствующего объекта
    connect(m_segmentCreationTool, &SegmentCreationTool::segmentDataReady, this, &MainWindow::createNewSegment);
    connect(m_propertiesPanel, &PropertiesPanelWidget::createSegmentRequested, this, &MainWindow::createNewSegment);

    //2) инструмент удаления сообщает о примитиве, который необходимо удалить -> в главном окне вызывается слот удаления объекта
    connect(m_deleteTool, &DeleteTool::primitiveHit, this, &MainWindow::deletePrimitive);

    //3) панель инструментов сообщает о нажатии кнопки -> в главном окне активируется соответствующий инструмент
    connect(m_toolbarPanel, &ToolbarPanelWidget::deleteToolActivated, this, &MainWindow::activateDeleteTool);
    connect(m_toolbarPanel, &ToolbarPanelWidget::segmentToolActivated, this, &MainWindow::activateSegmentCreationTool);

    //4) главное окно сообщает панели объектов, что сцена изменилась -> панель объектов обновляется
    connect(this, &MainWindow::sceneChanged, m_sceneObjectsPanel, &SceneObjectsPanelWidget::updateView);

    //5) главное окно сообщает панели свойств, что активирован инструмент -> панель свойств показывает пустую форму
    //QOverload используется, т.к. showPropertiesFor перегружен
    connect(this, &MainWindow::toolActivated, m_propertiesPanel, QOverload<PrimitiveType>::of(&PropertiesPanelWidget::showPropertiesFor));

    //6) главное окно сообщает панели свойств, что выбран объект -> сохраняется указатель на объект и панель свойств показывает его свойства
    connect(this, &MainWindow::objectSelected, this, [this](BasePrimitive* primitive) { m_selectedPrimitive = primitive; m_propertiesPanel->showPropertiesFor(primitive); });

    //7) панель объектов сцены сообщает, что пользователь выбрал примитив -> главное окно транслируем этот сигнал дальше (5 пункт)
    connect(m_sceneObjectsPanel, &SceneObjectsPanelWidget::primitiveSelected, this, &MainWindow::objectSelected);

    // Соединяем сигнал от новой панели со слотом во Viewport
    connect(m_sceneSettingsPanel, &SceneSettingsPanelWidget::gridSnapToggled, m_viewportPanel, &ViewportPanelWidget::setGridSnapEnabled); // <-- Добавьте эту строку

    connect(m_consolePanel, &ConsolePanelWidget::commandEntered, this, &MainWindow::processConsoleCommand); // <-- Добавить
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
    m_sceneSettingsPanel->updateIcons();
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

    QApplication::setOverrideCursor(Qt::CrossCursor);
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

void MainWindow::deletePrimitive(BasePrimitive* primitive)
{
    // Если нам не передали объект для удаления, ничего не делаем
    if (!primitive) {
        return;
    }

    // Удаляем объект из сцены
    m_scene->removePrimitive(primitive);

    // Если удаленный объект был тем, что мы выбрали в списке,
    // то "забываем" о нем.
    if (m_selectedPrimitive == primitive) {
        m_selectedPrimitive = nullptr;
    }

    emit sceneChanged(m_scene);
    m_viewportPanel->update();
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
        deletePrimitive(m_selectedPrimitive);
        return;
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

void MainWindow::processConsoleCommand(const QString& commandStr)
{
    ParsedCommand command = m_commandParser->parse(commandStr);

    if (!command.isValid) {
        // Здесь можно будет выводить ошибку в консоль
        qDebug("Неверный синтаксис команды.");
        return;
    }

    if (command.name == "segment" && command.args.size() == 4) {
        PointPrimitive start(command.args[0], command.args[1]);
        PointPrimitive end(command.args[2], command.args[3]);
        createNewSegment(start, end);
    } else {
        qDebug("Неизвестная команда или неверное количество аргументов.");
    }
}

void MainWindow::showEvent(QShowEvent* event)
{
    QMainWindow::showEvent(event);

    if (!m_isInitialResizeDone) {
        m_isInitialResizeDone = true;

        //определение размеров по ширине верхних и нижних колонок (процент от ширины экрана)
        //верхние колонки
        const double leftTopColumnPercentage = 0.20;
        const double rightTopColumnPercentage = 0.20;

        //нижние колонки
        const double leftDownColumnPercentage = 0.25;
        const double rightDownColumnPercentage = 0.25;

        //определение размеров колонок
        int totalWidth = width();
        int leftTopWidth = static_cast<int>(totalWidth * leftTopColumnPercentage);
        int rightTopWidth = static_cast<int>(totalWidth * rightTopColumnPercentage);
        int middleTopWidth = totalWidth - leftTopWidth - rightTopWidth;

        int leftDownWidth = static_cast<int>(totalWidth * leftDownColumnPercentage);
        int rightDownWidth = static_cast<int>(totalWidth * rightDownColumnPercentage);
        int middleDownWidth = totalWidth - leftDownWidth - rightDownWidth;

        resizeDocks({m_toolbarPanel, m_viewportPanel, m_sceneObjectsPanel}, {leftTopWidth, middleTopWidth, rightTopWidth}, Qt::Horizontal);
        resizeDocks({m_consolePanel, m_propertiesPanel, m_sceneSettingsPanel}, {leftDownWidth, middleDownWidth,rightDownWidth}, Qt::Horizontal);

        //определение размеров по высоте (процент от высоты экрана)
        const double toolbarHeightPercentage = 0.70;
        const double propertiesHeightPercentage = 0.30;

        int totalHeight = height();
        int toolbarHeight = static_cast<int>(totalHeight * toolbarHeightPercentage);
        int propertiesHeight = static_cast<int>(totalHeight * propertiesHeightPercentage);

        resizeDocks({m_toolbarPanel, m_propertiesPanel}, {toolbarHeight, propertiesHeight}, Qt::Vertical);
    }
}
