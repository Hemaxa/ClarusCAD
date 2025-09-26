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
#include "SettingsWindow.h"
#include "ThemeManager.h"

#include <QDockWidget>
#include <QMenuBar>
#include <QMenu>
#include <QKeyEvent>
#include <QCursor>
#include <QApplication>
#include <QToolButton>

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent), m_currentTool(nullptr)
{
    //настройки окна приложения
    setWindowTitle("ClarusCAD");
    setWindowState(Qt::WindowMaximized);
    setDockNestingEnabled(true);

    //применение сохраненной темы при запуске
    ThemeManager::instance().reloadTheme();

    //создание экземпляра сцены
    m_scene = new Scene();

    //создание парсера команд
    m_commandParser = new CommandParser(this);

    //вызов всех методов создания
    createTools();
    createDrawingTools();
    createPanelWindows();
    createConnections();
    createActions();
    createMenus();

    //пустой виджет панели параметров объекта
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
    m_propertiesPanel = new PropertiesPanelWidget("Свойства объекта", this);
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
    //1) инструмент и/или панель свойств сообщают данные -> в главном окне вызывается слот создания соответствующего объекта
    connect(m_segmentCreationTool, &SegmentCreationTool::segmentDataReady, this, &MainWindow::applySegmentChanges);
    connect(m_propertiesPanel, &PropertiesPanelWidget::segmentPropertiesApplied, this, &MainWindow::applySegmentChanges);

    //2) панель свойств сообщает об изменении цвета для нового объекта
    connect(m_propertiesPanel, &PropertiesPanelWidget::colorChanged, this, &MainWindow::onColorChanged);

    //3) инструмент удаления сообщает о примитиве, который необходимо удалить -> в главном окне вызывается слот удаления соответствующего объекта
    connect(m_deleteTool, &DeleteTool::primitiveHit, this, &MainWindow::deletePrimitive);

    //4) панель инструментов сообщает о нажатии кнопки -> в главном окне активируется соответствующий инструмент
    connect(m_toolbarPanel, &ToolbarPanelWidget::deleteToolActivated, this, &MainWindow::activateDeleteTool);
    connect(m_toolbarPanel, &ToolbarPanelWidget::segmentToolActivated, this, &MainWindow::activateSegmentCreationTool);

    //5) главное окно сообщает панели объектов, что сцена изменилась -> панель объектов сцены обновляется
    connect(this, &MainWindow::sceneChanged, m_sceneObjectsPanel, &SceneObjectsPanelWidget::updateView);

    //6) панель объектов сцены сообщает, что пользователь выбрал примитив -> главное окно транслируем этот сигнал дальше (7 пункт)
    connect(m_sceneObjectsPanel, &SceneObjectsPanelWidget::primitiveSelected, this, &MainWindow::objectSelected);

    //7) главное окно сообщает панели свойств, что активирован инструмент -> панель свойств показывает пустую форму
    connect(this, &MainWindow::toolActivated, m_propertiesPanel, QOverload<PrimitiveType>::of(&PropertiesPanelWidget::showPropertiesFor)); //QOverload используется, т.к. showPropertiesFor перегружен

    //8) главное окно сообщает панели свойств, что выбран объект -> сохраняется указатель на объект и панель свойств показывает его свойства
    connect(this, &MainWindow::objectSelected, this, [this](BasePrimitive* primitive) { m_selectedPrimitive = primitive; m_propertiesPanel->showPropertiesFor(primitive); });

    //9) панель параметров сцены сообщает об изменении настройки -> окно просмтора активирует соответствующий метод
    connect(m_sceneSettingsPanel, &SceneSettingsPanelWidget::gridSnapToggled, m_viewportPanel, &ViewportPanelWidget::setGridSnapEnabled);

    //10) панель параметров сцены сообщает об изменении системы координат -> панель свойств меняет содержимое
    connect(m_sceneSettingsPanel, &SceneSettingsPanelWidget::coordinateSystemChanged, m_propertiesPanel, &PropertiesPanelWidget::setCoordinateSystem);

    //11) панель консольного ввода сообщает о вводе команды -> главное окно запускает метод обработки команды
    connect(m_consolePanel, &ConsolePanelWidget::commandEntered, this, &MainWindow::processConsoleCommand);
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
    //создание окон и параметров
    m_settingsAction = new QAction("Настройки", this);
    m_settingsAction->setShortcut(QKeySequence::Preferences);
    m_settingsAction->setMenuRole(QAction::PreferencesRole);
    connect(m_settingsAction, &QAction::triggered, this, &MainWindow::openSettingsWindow);
}

void MainWindow::createMenus()
{
    //создание меню и добавление в них параметров
    QMenu* fileMenu = menuBar()->addMenu("Файл");
    fileMenu->addAction(m_settingsAction);
}

void MainWindow::activateDeleteTool()
{
    deactivateCurrentTool(); //дективация старого инструмента

    m_currentTool = m_deleteTool; //обновляется указатель на текущий инструмент
    m_viewportPanel->setActiveTool(m_currentTool); //окну просмотра передается информация о выбранном инструменте

    QApplication::setOverrideCursor(Qt::CrossCursor); //изменение курсора

    m_propertiesPanel->showPropertiesFor(nullptr);  //пустая панель свойств

    m_toolbarPanel->getDeleteButton()->setChecked(true); //установка кнопки в активное положение
}

void MainWindow::activateSegmentCreationTool()
{
    deactivateCurrentTool(); //дективация старого инструмента

    m_currentTool = m_segmentCreationTool; //обновляется указатель на текущий инструмент
    m_viewportPanel->setActiveTool(m_currentTool); //окну просмотра передается информация о выбранном инструменте

    emit toolActivated(PrimitiveType::Segment); //посылается сигнал о выборе инструмента
    m_toolbarPanel->getCreateSegmentButton()->setChecked(true); //установка кнопки в активное положение
}

void MainWindow::onColorChanged(const QColor& color)
{
    // Если какой-либо инструмент сейчас активен, передаем ему новый цвет
    if (m_currentTool) {
        m_currentTool->setColor(color);
    }
}

void MainWindow::applySegmentChanges(SegmentPrimitive* segment, const PointPrimitive& start, const PointPrimitive& end, const QColor& color)
{
    if (segment) {
        // РЕЖИМ РЕДАКТИРОВАНИЯ: обновляем существующий объект
        segment->setStart(start);
        segment->setEnd(end);
        segment->setColor(color);

        m_viewportPanel->update();
        emit sceneChanged(m_scene); // Обновляем список, но не меняем выбор
    } else {
        // РЕЖИМ СОЗДАНИЯ: создаем новый объект
        auto* newSegment = new SegmentPrimitive(start, end);
        newSegment->setColor(color);
        addPrimitiveToScene(newSegment); // Этот метод добавит объект и сделает его выбранным
    }
}

void MainWindow::deletePrimitive(BasePrimitive* primitive)
{
    //если не переан объект для удаления, ничего не удаляется
    if (!primitive) {
        return;
    }

    //удаляется объект из сцены
    m_scene->removePrimitive(primitive);

    //если удаленный объект был тем, который выбран в списке, то информация о нем забывается
    if (m_selectedPrimitive == primitive) {
        m_selectedPrimitive = nullptr;
    }

    //обновление окна просмотра
    emit sceneChanged(m_scene);
    m_viewportPanel->update();
}

void MainWindow::openSettingsWindow()
{
    SettingsWindow dialog(this);

    //передаются текущие настройки приложения
    dialog.setCurrentTheme(ThemeManager::instance().getThemeName());
    dialog.setGridStep(m_viewportPanel->getGridStep());
    dialog.setAngleUnit(PointPrimitive::getAngleUnit());

    //если нажата клавиша "ОК", используются новые значения из диалога
    if (dialog.exec() == QDialog::Accepted) {
        //применяется новая тема
        QString selectedTheme = dialog.getCurrentTheme();
        ThemeManager::instance().applyTheme(selectedTheme);

        //применяется новый шаг сетки
        int newGridStep = dialog.getGridStep();
        m_viewportPanel->setGridStep(newGridStep);

        //применяются новые единицы измерения углов
        AngleUnit newAngleUnit = dialog.getAngleUnit();
        PointPrimitive::setAngleUnit(newAngleUnit);

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
    //перенос команды в переменную
    ParsedCommand command = m_commandParser->parse(commandStr);

    //обработка некорректного синтаксиса команды
    if (!command.isValid) {
        qDebug("Неверный синтаксис команды.");
        return;
    }

    if (command.name == "segment" && command.args.size() == 4) {
        PointPrimitive start(command.args[0], command.args[1]);
        PointPrimitive end(command.args[2], command.args[3]);

        // --- ИЗМЕНЕНИЕ ЗДЕСЬ ---
        // Вызываем новый универсальный метод с nullptr, чтобы создать объект
        applySegmentChanges(nullptr, start, end, command.color);
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

        //расчет
        int totalWidth = width();
        int leftTopWidth = static_cast<int>(totalWidth * leftTopColumnPercentage);
        int rightTopWidth = static_cast<int>(totalWidth * rightTopColumnPercentage);
        int middleTopWidth = totalWidth - leftTopWidth - rightTopWidth;

        int leftDownWidth = static_cast<int>(totalWidth * leftDownColumnPercentage);
        int rightDownWidth = static_cast<int>(totalWidth * rightDownColumnPercentage);
        int middleDownWidth = totalWidth - leftDownWidth - rightDownWidth;

        //применение размеров
        resizeDocks({m_toolbarPanel, m_viewportPanel, m_sceneObjectsPanel}, {leftTopWidth, middleTopWidth, rightTopWidth}, Qt::Horizontal);
        resizeDocks({m_consolePanel, m_propertiesPanel, m_sceneSettingsPanel}, {leftDownWidth, middleDownWidth,rightDownWidth}, Qt::Horizontal);

        //определение размеров по высоте (процент от высоты экрана)
        const double toolbarHeightPercentage = 0.75;
        const double propertiesHeightPercentage = 0.25;

        //расчет
        int totalHeight = height();
        int toolbarHeight = static_cast<int>(totalHeight * toolbarHeightPercentage);
        int propertiesHeight = static_cast<int>(totalHeight * propertiesHeightPercentage);

        //применение размеров
        resizeDocks({m_toolbarPanel, m_propertiesPanel}, {toolbarHeight, propertiesHeight}, Qt::Vertical);
    }
}
