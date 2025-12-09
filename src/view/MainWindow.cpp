#include "MainWindow.h"
#include "Scene.h"

#include "DeleteTool.h"
#include "MoveTool.h"
#include "SegmentCreationTool.h"

#include "SegmentPrimitive.h"

#include "ConsolePanelWidget.h"
#include "NavigationPanelWidget.h"
#include "PropertiesPanelWidget.h"
#include "SceneObjectsPanelWidget.h"
#include "SceneSettingsPanelWidget.h"
#include "ToolbarPanelWidget.h"
#include "ViewportPanelWidget.h"

#include "SettingsWindow.h"
#include "LineStyleManager.h"
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

    //применение выбранной толщины линий и параметров штриховки
    LineStyleManager::instance().setBaseLineThickness(SettingsManager::instance().getBaseLineThickness());
    LineStyleManager::instance().setDashLength(SettingsManager::instance().getDashLength());
    LineStyleManager::instance().setDashSpace(SettingsManager::instance().getDashSpace());

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
    m_propertiesPanel->showPropertiesFor(QList<BasePrimitive*>());
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
    m_navigationPanel = new NavigationPanelWidget("Навигация", this);
    m_propertiesPanel = new PropertiesPanelWidget("Свойства объекта", this);
    m_sceneObjectsPanel = new SceneObjectsPanelWidget("Список объектов", this);
    m_sceneSettingsPanel = new SceneSettingsPanelWidget("Параметры сцены", this);
    m_consolePanel = new ConsolePanelWidget("Консольный ввод", this);

    //установка имен панелей для оформления из файлов тем
    m_viewportPanel->setProperty("class", "ViewportPanel");
    m_toolbarPanel->setProperty("class", "ToolbarPanel");
    m_navigationPanel->setProperty("class", "NavigationPanel");
    m_propertiesPanel->setProperty("class", "PropertiesPanel");
    m_sceneObjectsPanel->setProperty("class", "SceneObjectsPanel");
    m_sceneSettingsPanel->setProperty("class", "SceneSettingsPanel");
    m_consolePanel->setProperty("class", "ConsolePanel");

    //передача сцены в панель просмотра
    m_viewportPanel->setScene(m_scene);

    //расстановка панелей интерфейса
    //распределение столбцов
    addDockWidget(Qt::LeftDockWidgetArea, m_toolbarPanel);
    addDockWidget(Qt::RightDockWidgetArea, m_sceneObjectsPanel);

    splitDockWidget(m_toolbarPanel, m_viewportPanel, Qt::Horizontal);
    splitDockWidget(m_viewportPanel, m_sceneObjectsPanel, Qt::Horizontal);

    //распределение строк
    splitDockWidget(m_viewportPanel, m_propertiesPanel, Qt::Vertical);

    splitDockWidget(m_toolbarPanel, m_navigationPanel, Qt::Vertical);
    splitDockWidget(m_navigationPanel, m_consolePanel, Qt::Vertical);

    splitDockWidget(m_sceneObjectsPanel, m_sceneSettingsPanel, Qt::Vertical);
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
    connect(m_toolbarPanel, &ToolbarPanelWidget::deleteToolActivated, this, &MainWindow::activateDeleteTool);
    connect(m_toolbarPanel, &ToolbarPanelWidget::moveToolActivated, this, &MainWindow::activateMoveTool);
    connect(m_toolbarPanel, &ToolbarPanelWidget::segmentToolActivated, this, &MainWindow::activateSegmentCreationTool);

    //- NavigationPanelWidget -
    connect(m_navigationPanel, &NavigationPanelWidget::zoomInClicked, m_viewportPanel, QOverload<>::of(&ViewportPanelWidget::zoomIn));
    connect(m_navigationPanel, &NavigationPanelWidget::zoomOutClicked, m_viewportPanel, QOverload<>::of(&ViewportPanelWidget::zoomOut));
    connect(m_navigationPanel, &NavigationPanelWidget::zoomExtentsClicked, this, &MainWindow::onZoomExtents);
    connect(m_navigationPanel, &NavigationPanelWidget::rotateCWClicked, this, &MainWindow::onRotateLeft);
    connect(m_navigationPanel, &NavigationPanelWidget::rotateCCWClicked, this, &MainWindow::onRotateRight);

    //- SceneSettingsPanelWidget -
    connect(m_sceneSettingsPanel, &SceneSettingsPanelWidget::gridSnapToggled, m_viewportPanel, &ViewportPanelWidget::setGridSnapEnabled);
    connect(m_sceneSettingsPanel, &SceneSettingsPanelWidget::primitiveSnapToggled, m_viewportPanel, &ViewportPanelWidget::setPrimitiveSnapEnabled);

    connect(m_sceneSettingsPanel, &SceneSettingsPanelWidget::coordinateSystemChanged, m_propertiesPanel, &PropertiesPanelWidget::setCoordinateSystem);
    connect(m_sceneSettingsPanel, &SceneSettingsPanelWidget::coordinateSystemChanged, m_viewportPanel, &ViewportPanelWidget::setCoordinateSystem);

    //- SceneObjectsPanelWidget -
    connect(m_sceneObjectsPanel, &SceneObjectsPanelWidget::primitivesSelected, this, &MainWindow::onSelectionChanged);

    //- ConsolePanelWidget -
    connect(m_consolePanel, &ConsolePanelWidget::commandParsed, this, &MainWindow::onConsoleCommandParsed);

    //- Tools -
    connect(m_segmentCreationTool, &SegmentCreationTool::segmentDataReady, this, &MainWindow::applySegmentChanges);
    connect(m_deleteTool, &DeleteTool::primitiveHit, this, &MainWindow::deletePrimitive);
    connect(m_viewportPanel, &ViewportPanelWidget::mouseMoved, m_moveTool, &MoveTool::updateMousePosition);

    //- ViewportPanelWidget -
    connect(m_viewportPanel, &ViewportPanelWidget::selectionChanged, this, &MainWindow::onSelectionChanged);

    //- Managers -
    //менеджер настроек сообщает об изменении настройки -> компоненты обновляются
    connect(&SettingsManager::instance(), &SettingsManager::gridStepChanged, m_viewportPanel, &ViewportPanelWidget::setGridStep);
    connect(&SettingsManager::instance(), &SettingsManager::zoomStepChanged, m_viewportPanel, &ViewportPanelWidget::setZoomStep);
    connect(&SettingsManager::instance(), &SettingsManager::angleUnitChanged, &PointPrimitive::setAngleUnit);
    connect(&SettingsManager::instance(), &SettingsManager::themeNameChanged, &ThemeManager::instance(), &ThemeManager::applyTheme);
    connect(&SettingsManager::instance(), &SettingsManager::baseLineThicknessChanged, &LineStyleManager::instance(), &LineStyleManager::setBaseLineThickness);

    // Новые подключения для параметров штриховки
    connect(&SettingsManager::instance(), &SettingsManager::dashLengthChanged, &LineStyleManager::instance(), &LineStyleManager::setDashLength);
    connect(&SettingsManager::instance(), &SettingsManager::dashSpaceChanged, &LineStyleManager::instance(), &LineStyleManager::setDashSpace);

    //менеджер линий сообщает об изменении стиля линий -> компоненты обновляются
    connect(&LineStyleManager::instance(), &LineStyleManager::stylesChanged, m_viewportPanel, QOverload<>::of(&ViewportPanelWidget::update));

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

void MainWindow::onZoomExtents()
{
    m_viewportPanel->zoomToExtents();
}

void MainWindow::onRotateLeft()
{
    m_viewportPanel->rotateSceneLeft();
}

void MainWindow::onRotateRight()
{
    m_viewportPanel->rotateSceneRight();
}

void MainWindow::onSelectionChanged(const QList<BasePrimitive*>& primitives)
{
    // Сохраняем список выделенного
    m_selectedPrimitives = primitives;

    // Передаем список обратно во вьюпорт (чтобы он подсветил их зеленым)
    // Это важно, если выделение пришло из списка объектов, а не из вьюпорта
    m_viewportPanel->setSelectedPrimitives(primitives);

    // Обновляем панель свойств
    if (primitives.isEmpty()) {
        m_propertiesPanel->showPropertiesFor(QList<BasePrimitive*>());
    }
    else {
        // Передаем весь список, PropertiesPanelWidget сам разберется, как их отобразить
        m_propertiesPanel->showPropertiesFor(primitives);
    }
}

void MainWindow::onConsoleCommandParsed(const ParsedCommand& command)
{
    //обработка разных команд
    if (command.name == "segment" && command.args.size() == 4) {
        PointPrimitive start(command.args[0], command.args[1]);
        PointPrimitive end(command.args[2], command.args[3]);
        applySegmentChanges(nullptr, start, end, command.color, LineType::SolidMain); //команды из консоли пока будут сплошными линиями
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
        m_propertiesPanel->showPropertiesFor(QList<BasePrimitive*>()); //сброс панель свойств
        m_viewportPanel->setActiveTool(nullptr); //сброс активного инструмента в окне просмотра
        m_viewportPanel->getCanvas()->setCursor(Qt::ArrowCursor); //установка стандартного курсора
        m_viewportPanel->update(); //обновление окна просмотра
    }
}



//--- МЕТОДЫ ВЗАИМОДЕЙСТВИЯ С ОБЪЕКТАМИ ---
void MainWindow::applySegmentChanges(SegmentPrimitive* segment, const PointPrimitive& start, const PointPrimitive& end, const QColor& color, LineType lineType)
{
    // Режим создания нового (segment == nullptr)
    if (!segment) {
        auto* newSegment = new SegmentPrimitive(start, end);
        newSegment->setColor(color);
        newSegment->setLineType(static_cast<int>(lineType)); // Приведение к int
        addPrimitiveToScene(newSegment);
        return;
    }

    // Режим редактирования (segment != nullptr)

    // Если редактируемый объект входит в группу выделенных, применяем общие свойства ко всем
    if (m_selectedPrimitives.contains(segment) && m_selectedPrimitives.size() > 1) {
        for (auto* prim : m_selectedPrimitives) {
            // Применяем общие визуальные свойства
            // Для корректной работы с разными цветами: если пришел Qt::white (наш "смешанный" флаг),
            // но мы не должны его сбрасывать? Нет, логика в PropertiesWidget посылает конкретный цвет,
            // если пользователь его выбрал. Если пользователь не трогал цвет, Widget должен прислать
            // либо старый цвет segment-а, либо флаг. В текущей реализации Widget шлет m_selectedColor.
            // Если он был смешанный, там Qt::white. Это может быть проблемой (сброс в белый).
            // Но пока оставим простую логику: что пришло, то и ставим.

            prim->setColor(color);
            prim->setLineType(static_cast<int>(lineType));

            // Геометрию (start/end) обычно при массовом редактировании не трогают,
            // либо меняют только у "главного". В данном случае меняем только у segment.
        }

        // Главному (отображаемому в панели) меняем всё
        segment->setStart(start);
        segment->setEnd(end);
    }
    else {
        // Одиночное редактирование
        segment->setStart(start);
        segment->setEnd(end);
        segment->setColor(color);
        segment->setLineType(static_cast<int>(lineType));
    }

    // Сигнал об изменении сцены обновит список объектов
    emit sceneChanged(m_scene);

    // Примечание: Мы не сбрасываем m_selectedPrimitives здесь,
    // чтобы выделение осталось после перерисовки.
}

void MainWindow::addPrimitiveToScene(BasePrimitive* primitive)
{
    if (primitive) {
        m_scene->addPrimitive(std::unique_ptr<BasePrimitive>(primitive));

        // При создании нового объекта выделяем только его
        QList<BasePrimitive*> newSelection;
        newSelection.append(primitive);

        emit sceneChanged(m_scene); // Обновляет список и вьюпорт

        // Принудительно устанавливаем выделение нового объекта
        onSelectionChanged(newSelection);
    }
}

void MainWindow::deletePrimitive(BasePrimitive* primitive)
{
    // Если есть выделенные объекты (удаление по кнопке Del или через меню)
    if (!m_selectedPrimitives.isEmpty()) {
        for (auto* prim : m_selectedPrimitives) {
            m_scene->removePrimitive(prim);
        }
        m_selectedPrimitives.clear();

        // Сбрасываем выделение в UI
        onSelectionChanged(m_selectedPrimitives);
        emit sceneChanged(m_scene);
        return;
    }

    // Если удаляем конкретный примитив (например, инструментом Ластик)
    if (primitive) {
        m_scene->removePrimitive(primitive);
        emit sceneChanged(m_scene);
    }
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
    if ((event->key() == Qt::Key_Delete || event->key() == Qt::Key_Backspace) && !m_selectedPrimitives.isEmpty()) {
        deletePrimitive(nullptr); // Вызываем без аргумента, чтобы сработала ветка удаления списка
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

// void MainWindow::showEvent(QShowEvent* event)
// {
//     QMainWindow::showEvent(event);

//     if (!m_isInitialResizeDone) {
//         m_isInitialResizeDone = true;

//         //определение размеров по ширине верхних и нижних колонок (процент от ширины экрана)
//         //верхние колонки
//         const double leftTopColumnPercentage = 0.20;
//         const double rightTopColumnPercentage = 0.20;

//         //нижние колонки
//         const double leftDownColumnPercentage = 0.25;
//         const double rightDownColumnPercentage = 0.25;

//         //расчет
//         int totalWidth = width();
//         int leftTopWidth = static_cast<int>(totalWidth * leftTopColumnPercentage);
//         int rightTopWidth = static_cast<int>(totalWidth * rightTopColumnPercentage);
//         int middleTopWidth = totalWidth - leftTopWidth - rightTopWidth;

//         int leftDownWidth = static_cast<int>(totalWidth * leftDownColumnPercentage);
//         int rightDownWidth = static_cast<int>(totalWidth * rightDownColumnPercentage);
//         int middleDownWidth = totalWidth - leftDownWidth - rightDownWidth;

//         //применение размеров
//         resizeDocks({m_toolbarPanel, m_viewportPanel, m_sceneObjectsPanel}, {leftTopWidth, middleTopWidth, rightTopWidth}, Qt::Horizontal);
//         resizeDocks({m_consolePanel, m_propertiesPanel, m_sceneSettingsPanel}, {leftDownWidth, middleDownWidth,rightDownWidth}, Qt::Horizontal);

//         //определение размеров по высоте (процент от высоты экрана)
//         const double toolbarHeightPercentage = 0.75;
//         const double propertiesHeightPercentage = 0.25;

//         //расчет
//         int totalHeight = height();
//         int toolbarHeight = static_cast<int>(totalHeight * toolbarHeightPercentage);
//         int propertiesHeight = static_cast<int>(totalHeight * propertiesHeightPercentage);

//         //применение размеров
//         resizeDocks({m_toolbarPanel, m_navigationPanel, m_propertiesPanel}, {toolbarHeight, propertiesHeight}, Qt::Vertical);
//     }
// }

void MainWindow::showEvent(QShowEvent* event)
{
    QMainWindow::showEvent(event);

    if (!m_isInitialResizeDone) {
        m_isInitialResizeDone = true;

        int totalWidth = width();
        int totalHeight = height();

        //настройка горизонтальных пропорций
        const double leftColumnPercentage = 0.15;
        const double rightColumnPercentage = 0.15;

        int leftColWidth = static_cast<int>(totalWidth * leftColumnPercentage);
        int rightColWidth = static_cast<int>(totalWidth * rightColumnPercentage);
        int centerColWidth = totalWidth - leftColWidth - rightColWidth;

        resizeDocks({m_toolbarPanel, m_viewportPanel, m_sceneObjectsPanel}, {leftColWidth, centerColWidth, rightColWidth}, Qt::Horizontal);

        //настройка вертикальных пропорций
        //левая колонка
        int toolbarHeight = static_cast<int>(totalHeight * 0.4);
        int navHeight = static_cast<int>(totalHeight * 0.36);
        int consoleHeight = totalHeight - toolbarHeight - navHeight;

        resizeDocks({m_toolbarPanel, m_navigationPanel, m_consolePanel}, {toolbarHeight, navHeight, consoleHeight}, Qt::Vertical);

        //центральная колонка
        int viewportHeight = static_cast<int>(totalHeight * 0.8);
        int propertiesHeight = totalHeight - viewportHeight;

        resizeDocks({m_viewportPanel, m_propertiesPanel}, {viewportHeight, propertiesHeight}, Qt::Vertical);

        //правая колонка
        int sceneObjectsHeight = static_cast<int>(totalHeight * 0.76);
        int sceneSettingsHeight = totalHeight - sceneObjectsHeight;

        resizeDocks({m_sceneObjectsPanel, m_sceneSettingsPanel}, {sceneObjectsHeight, sceneSettingsHeight}, Qt::Vertical);
    }
}
