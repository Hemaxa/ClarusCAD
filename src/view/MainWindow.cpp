#include "MainWindow.h"
#include "ViewportWidget.h"
#include "Scene.h"
#include "LinePropertiesWidget.h"

#include <QDockWidget>
#include <QListWidget>
#include <QLabel>
#include <QToolBar>
#include <QAction>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("ClarusCAD");
    resize(1280, 720);
    setDockNestingEnabled(true);

    m_scene = new Scene();

    // Создаем виджеты
    m_viewportWidget = new ViewportWidget();
    m_viewportWidget->setScene(m_scene);
    m_sceneObjectsList = new QListWidget();

    // Создаем наш новый виджет свойств
    m_propertiesWidget = new LinePropertiesWidget();
    m_bottomRightWidget = new QLabel("Эта область пока свободна");

    // Создаем док-виджеты
    createDockWindows();

    // Создаем панель инструментов (Toolbar)
    QToolBar* mainToolBar = new QToolBar("Инструменты", this);
    addToolBar(mainToolBar);

    // Добавляем действие (кнопку) для создания отрезка
    QAction* createLineAction = new QAction("Создать отрезок", this);
    // В будущем сюда можно добавить иконку: createLineAction->setIcon(...)
    mainToolBar->addAction(createLineAction);

    // Соединяем сигналы и слоты
    connect(createLineAction, &QAction::triggered, this, &MainWindow::showLineCreationTool);
    connect(m_propertiesWidget, &LinePropertiesWidget::createSegmentRequested, this, &MainWindow::handleCreateSegment);
}

MainWindow::~MainWindow() {
    delete m_scene;
}

void MainWindow::createDockWindows()
{
    // Убираем центральный виджет, чтобы все пространство делили доки
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

    QDockWidget *bottomRightDock = new QDockWidget("Дополнительно", this);
    bottomRightDock->setWidget(m_bottomRightWidget);
    addDockWidget(Qt::RightDockWidgetArea, bottomRightDock);

    // Теперь "склеим" их, чтобы получить сетку 2x2
    splitDockWidget(viewportDock, propertiesDock, Qt::Vertical);
    splitDockWidget(sceneListDock, bottomRightDock, Qt::Vertical);
}

void MainWindow::handleCreateSegment(const Point& start, const Point& end)
{
    // 1. Создаем объект отрезка и добавляем его в сцену
    Segment segment(start, end);
    m_scene->addSegment(segment);

    // 2. Обновляем список объектов на сцене
    m_sceneObjectsList->addItem(QString("Отрезок (%1, %2) - (%3, %4)")
                                    .arg(start.x()).arg(start.y()).arg(end.x()).arg(end.y()));

    // 3. Перерисовываем рабочую область, чтобы увидеть результат
    m_viewportWidget->update();
}

void MainWindow::showLineCreationTool()
{
    // Эта функция может быть сложнее в будущем
    // Например, она может менять видимость разных виджетов свойств.
    // Пока просто убедимся, что док-виджет с параметрами виден.
    m_propertiesWidget->parentWidget()->show();
    m_propertiesWidget->parentWidget()->raise(); // И поднимаем его наверх
}
