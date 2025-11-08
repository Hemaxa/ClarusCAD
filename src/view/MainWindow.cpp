#include "MainWindow.h"
#include "Scene.h"

#include "DeleteTool.h"
#include "MoveTool.h"
#include "SegmentCreationTool.h"

#include "SegmentPrimitive.h"

#include "ConsolePanelWidget.h"
#include "PropertiesPanelWidget.h"
#include "SceneObjectsPanelWidget.h"
#include "SceneSettingsPanelWidget.h"
#include "ToolbarPanelWidget.h"
#include "ViewportPanelWidget.h"

#include "SettingsWindow.h"
#include "ThemeManager.h"
#include "SettingsManager.h"

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

    //загрузка сохраненных настроек
    SettingsManager::instance().loadSettings();

    //применение выбранной темы
    ThemeManager::instance().applyTheme(SettingsManager::instance().getThemeName());

    //создание экземпляра сцены
    m_scene = new Scene();

    //вызов всех методов создания
    createTools();
    createPanelWindows();
    createConnections();
    createActions();
    createMenus();

    //применение первоначальных настроек
    m_viewportPanel->setGridStep(SettingsManager::instance().getGridStep());
    m_viewportPanel->setZoomStep(SettingsManager::instance().getZoomStep());
    PointPrimitive::setAngleUnit(SettingsManager::instance().getAngleUnit());

    //пустой виджет панели параметров объекта
    m_propertiesPanel->showPropertiesFor(nullptr);
}

MainWindow::~MainWindow()
{
    delete m_scene;
}



//--- МЕТОДЫ СОЗДАНИЯ ---
void MainWindow::createTools()
{
    //создание экземпляров инструментов
    m_deleteTool = new DeleteTool(this);
    m_moveTool = new MoveTool(this);
    m_segmentCreationTool = new SegmentCreationTool(this);
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

    //установка имен панелей для оформления из файлов тем
    m_viewportPanel->setProperty("class", "ViewportPanel");
    m_toolbarPanel->setProperty("class", "ToolbarPanel");
    m_propertiesPanel->setProperty("class", "PropertiesPanel");
    m_sceneObjectsPanel->setProperty("class", "SceneObjectsPanel");
    m_sceneSettingsPanel->setProperty("class", "SceneSettingsPanel");
    m_consolePanel->setProperty("class", "ConsolePanel");

    //передача сцены в панель просмотра
    m_viewportPanel->setScene(m_scene);

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
    //- MainWindow -
    //главное окно сообщает панели объектов, что сцена изменилась -> панель объектов сцены обновляется
    connect(this, &MainWindow::sceneChanged, m_sceneObjectsPanel, &SceneObjectsPanelWidget::update);

    //главное окно сообщает окну просмотра, что сцена изменилась -> окно просмотра перерисовывается
    connect(this, &MainWindow::sceneChanged, m_viewportPanel, &ViewportPanelWidget::update);

    //главное окно сообщает панели свойств, что активирован инструмент создания -> панель свойств показывает пустую форму в зависимости от типа примитива
    connect(this, &MainWindow::toolActivated, m_propertiesPanel, QOverload<PrimitiveType>::of(&PropertiesPanelWidget::showPropertiesFor));

    //- PropertiesPanelWidget -
    //панель свойств сообщает данные -> в главном окне вызывается слот создания соответствующего примитива
    connect(m_propertiesPanel, &PropertiesPanelWidget::segmentPropertiesApplied, this, &MainWindow::applySegmentChanges);

    //панель свойств сообщает об изменении данных (параметров) при создании нового примитива -> в главном окне вызываются слот установки параметра
    connect(m_propertiesPanel, &PropertiesPanelWidget::colorChanged, this, &MainWindow::onColorChanged);
    connect(m_propertiesPanel, &PropertiesPanelWidget::lineTypeChanged, this, &MainWindow::onLineTypeChanged);

    //- ToolbarPanelWidget -
    //панель инструментов сообщает о нажатии кнопки -> в главном окне активируется соответствующий инструмент
    connect(m_toolbarPanel, &ToolbarPanelWidget::deleteToolActivated, this, &MainWindow::activateDeleteTool);
    connect(m_toolbarPanel, &ToolbarPanelWidget::moveToolActivated, this, &MainWindow::activateMoveTool);
    connect(m_toolbarPanel, &ToolbarPanelWidget::segmentToolActivated, this, &MainWindow::activateSegmentCreationTool);

    //- SceneSettingsPanelWidget -
    //панель параметров сцены сообщает об изменении настройки -> окно просмтора активирует соответствующий метод
    connect(m_sceneSettingsPanel, &SceneSettingsPanelWidget::gridSnapToggled, m_viewportPanel, &ViewportPanelWidget::setGridSnapEnabled);
    connect(m_sceneSettingsPanel, &SceneSettingsPanelWidget::primitiveSnapToggled, m_viewportPanel, &ViewportPanelWidget::setPrimitiveSnapEnabled);

    //панель параметров сцены сообщает о нажатии кнопки -> окно просмтора активирует соответствующий метод
    connect(m_sceneSettingsPanel, &SceneSettingsPanelWidget::zoomInClicked, m_viewportPanel, QOverload<>::of(&ViewportPanelWidget::zoomIn));
    connect(m_sceneSettingsPanel, &SceneSettingsPanelWidget::zoomOutClicked, m_viewportPanel, QOverload<>::of(&ViewportPanelWidget::zoomOut));

    //панель параметров сцены сообщает об изменении системы координат -> панель свойств и окна просмотра меняют содержимое
    connect(m_sceneSettingsPanel, &SceneSettingsPanelWidget::coordinateSystemChanged, m_propertiesPanel, &PropertiesPanelWidget::setCoordinateSystem);
    connect(m_sceneSettingsPanel, &SceneSettingsPanelWidget::coordinateSystemChanged, m_viewportPanel, &ViewportPanelWidget::setCoordinateSystem);

    //- SceneObjectsPanelWidget -
    //панель объектов сцены сообщает, что пользователь выбрал примитив -> в главном окне вызывается слот изменения выбранного примитива
    connect(m_sceneObjectsPanel, &SceneObjectsPanelWidget::primitiveSelected, this, &MainWindow::onSelectionChanged);

    //- ConsolePanelWidget -
    //панель консольного ввода сообщает о вводе команды -> главное окно вызывает слот обработки консольной команды
    connect(m_consolePanel, &ConsolePanelWidget::commandParsed, this, &MainWindow::onConsoleCommandParsed);

    //- Tools -
    //инструмент создания сообщает данные -> в главном окне вызывается слот создания соответствующего примитива
    connect(m_segmentCreationTool, &SegmentCreationTool::segmentDataReady, this, &MainWindow::applySegmentChanges);

    //инструмент удаления сообщает о примитиве, который необходимо удалить -> в главном окне вызывается слот удаления соответствующего объекта
    connect(m_deleteTool, &DeleteTool::primitiveHit, this, &MainWindow::deletePrimitive);

    //панель просмотра сцены сообщает о движении мыши -> инструмент перемещения обновляет свою позицию
    connect(m_viewportPanel, &ViewportPanelWidget::mouseMoved, m_moveTool, &MoveTool::updateMousePosition);

    //- Managers -
    //менеджер настроек сообщает об изменении настройки -> компоненты обновляются
    connect(&SettingsManager::instance(), &SettingsManager::gridStepChanged, m_viewportPanel, &ViewportPanelWidget::setGridStep);
    connect(&SettingsManager::instance(), &SettingsManager::zoomStepChanged, m_viewportPanel, &ViewportPanelWidget::setZoomStep);
    connect(&SettingsManager::instance(), &SettingsManager::angleUnitChanged, &PointPrimitive::setAngleUnit);
    connect(&SettingsManager::instance(), &SettingsManager::themeNameChanged, &ThemeManager::instance(), &ThemeManager::applyTheme);

    //менеджер тем сообщает об изменении темы -> панели обновляются
    connect(&ThemeManager::instance(), &ThemeManager::themeApplied, m_toolbarPanel, &ToolbarPanelWidget::updateColors);
    connect(&ThemeManager::instance(), &ThemeManager::themeApplied, m_sceneSettingsPanel, &SceneSettingsPanelWidget::updateColors);
    connect(&ThemeManager::instance(), &ThemeManager::themeApplied, m_propertiesPanel, &PropertiesPanelWidget::updateColors);
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



//--- МЕТОДЫ ИЗМЕНЕНИЯ СОСТОЯНИЙ ---
void MainWindow::onColorChanged(const QColor& color)
{
    //если какой-либо инструмент сейчас активен, ему передается новый цвет
    if (m_currentTool) {
        m_currentTool->setColor(color);
    }
}

void MainWindow::onLineTypeChanged(LineType type)
{
    //если какой-либо инструмент сейчас активен, ему передается новый тип линии
    if (m_currentTool) {
        m_currentTool->setLineType(type);
    }
}

void MainWindow::onSelectionChanged(BasePrimitive* primitive)
{
    //меняется указатель на примитив и вызываются методы в необходимых панелях
    m_selectedPrimitive = primitive;
    m_propertiesPanel->showPropertiesFor(primitive);
    m_viewportPanel->setSelectedPrimitive(primitive);
}

void MainWindow::onConsoleCommandParsed(const ParsedCommand& command)
{
    //обработка разных команд
    if (command.name == "segment" && command.args.size() == 4) {
        PointPrimitive start(command.args[0], command.args[1]);
        PointPrimitive end(command.args[2], command.args[3]);
        applySegmentChanges(nullptr, start, end, command.color, LineType::Solid); //команды из консоли пока будут сплошными линиями
    }
    else {
        qDebug("Неизвестная команда или неверное количество аргументов.");
    }
}



//--- МЕТОДЫ АКТИВАЦИИ/ДЕАКТИВАЦИИ ИНСТРУМЕНТОВ ---
void MainWindow::activateDeleteTool()
{
    deactivateCurrentTool(); //дективация старого инструмента

    m_currentTool = m_deleteTool; //обновляется указатель на текущий инструмент
    m_viewportPanel->setActiveTool(m_currentTool); //окну просмотра передается информация о выбранном инструменте

    m_viewportPanel->getCanvas()->setCursor(Qt::CrossCursor); //изменение курсора

    emit toolActivated(PrimitiveType::Generic); //показывается пустая панель свойств

    m_toolbarPanel->getDeleteButton()->setChecked(true); //установка кнопки в активное положение
}

void MainWindow::activateMoveTool()
{
    deactivateCurrentTool();

    m_currentTool = m_moveTool;
    m_viewportPanel->setActiveTool(m_currentTool);

    m_moveTool->activate(m_viewportPanel); //инструмент активиурется

    m_viewportPanel->getCanvas()->setCursor(Qt::OpenHandCursor);

    emit toolActivated(PrimitiveType::Generic);

    m_toolbarPanel->getMoveButton()->setChecked(true);
}

void MainWindow::activateSegmentCreationTool()
{
    deactivateCurrentTool(); //дективация старого инструмента

    m_currentTool = m_segmentCreationTool; //обновляется указатель на текущий инструмент
    m_viewportPanel->setActiveTool(m_currentTool); //окну просмотра передается информация о выбранном инструменте

    emit toolActivated(PrimitiveType::Segment); //посылается сигнал о выборе инструмента

    m_toolbarPanel->getCreateSegmentButton()->setChecked(true); //установка кнопки в активное положение
}

void MainWindow::deactivateCurrentTool()
{
    //если какой-либо инструмент активен
    if (m_currentTool) {
        m_currentTool->reset(); //сбрасывается состояние инструмента
        m_currentTool = nullptr; //сбрасывается указатель на инструмент
        m_toolbarPanel->clearSelection(); //кнопка на панели инструментов "отжимается"
        m_propertiesPanel->showPropertiesFor(nullptr); //сброс панель свойств
        m_viewportPanel->setActiveTool(nullptr); //сброс активного инструмента в окне просмотра
        m_viewportPanel->getCanvas()->setCursor(Qt::ArrowCursor); //установка стандартного курсора
        m_viewportPanel->update(); //обновление окна просмотра
    }
}



//--- МЕТОДЫ ВЗАИМОДЕЙСТВИЯ С ОБЪЕКТАМИ ---
void MainWindow::applySegmentChanges(SegmentPrimitive* segment, const PointPrimitive& start, const PointPrimitive& end, const QColor& color, LineType lineType)
{
    //режим редактирования
    if (segment) {
        //обновляется существующий объект
        segment->setStart(start);
        segment->setEnd(end);
        segment->setColor(color);
        segment->setLineType(lineType);

        emit sceneChanged(m_scene); //обновит список, но не изменит выбор
    }
    //режим создания
    else {
        //создается новый объект
        auto* newSegment = new SegmentPrimitive(start, end);
        newSegment->setColor(color);
        newSegment->setLineType(lineType);
        addPrimitiveToScene(newSegment); //добавит объект и сделает его выбранным
    }
}

void MainWindow::addPrimitiveToScene(BasePrimitive* primitive)
{
    if (primitive) {
        m_scene->addPrimitive(std::unique_ptr<BasePrimitive>(primitive)); //добавление примитива в вектор сцены

        emit sceneChanged(m_scene); //посылается сигнал, что сцена изменилась
        onSelectionChanged(primitive); //посылается сигнал, о выбранном объекте
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
        onSelectionChanged(nullptr);
    }

    //обновление окна просмотра
    emit sceneChanged(m_scene);
}



//--- МЕТОДЫ ПЕРЕХВАТА ДЕЙСТВИЙ С КЛАВИАТУРЫ ---
void MainWindow::keyPressEvent(QKeyEvent* event)
{
    //1) space
    if (event->key() == Qt::Key_Space && !event->isAutoRepeat()) {
        if (m_currentTool == m_moveTool) {
            return;
        }
        //активируется режим перемещения
        if (!m_moveTool->isActive()) {
            m_previousCursor = m_viewportPanel->getCanvas()->cursor();
            m_moveTool->activate(m_viewportPanel);
            m_viewportPanel->getCanvas()->setCursor(Qt::OpenHandCursor);
        }
        return;
    }

    //2) esc
    if (event->key() == Qt::Key_Escape) {
        deactivateCurrentTool();
        return;
    }

    //3) delete / backspace
    if ((event->key() == Qt::Key_Delete || event->key() == Qt::Key_Backspace) && m_selectedPrimitive) {
        deletePrimitive(m_selectedPrimitive);
        return;
    }

    //4) cmd + / cmd -
    bool cursorIsOverViewport = m_viewportPanel->underMouse();

    if (cursorIsOverViewport) {
        if ((event->key() == Qt::Key_Plus || event->key() == Qt::Key_Equal) && (event->modifiers() & Qt::MetaModifier)) {
            //считается позиция курсора относительно холста окна просмотра
            QPoint mousePos = m_viewportPanel->getCanvas()->mapFromGlobal(QCursor::pos());
            m_viewportPanel->zoomIn(mousePos);
            return;
        }
        if (event->key() == Qt::Key_Minus && (event->modifiers() & Qt::MetaModifier)) {
            QPoint mousePos = m_viewportPanel->getCanvas()->mapFromGlobal(QCursor::pos());
            m_viewportPanel->zoomOut(mousePos);
            return;
        }
    }
    QMainWindow::keyPressEvent(event);
}

void MainWindow::keyReleaseEvent(QKeyEvent* event)
{
    // отжатие space
    if (event->key() == Qt::Key_Space && !event->isAutoRepeat()) {
        if (m_currentTool != m_moveTool) {
            m_moveTool->deactivate();
            m_viewportPanel->getCanvas()->setCursor(m_previousCursor); //восстанавливается старый курсор
        }
        return;
    }
    QMainWindow::keyReleaseEvent(event);
}



//--- ОСТАЛЬНОЕ ---
void MainWindow::openSettingsWindow()
{
    SettingsWindow dialog(this);
    dialog.exec();
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
