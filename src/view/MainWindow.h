//MainWindow - класс, который владеет, компонует и связвыет остальные классы между собой

#pragma once

#include "PointPrimitive.h"
#include "BasePrimitive.h"

#include <QMainWindow>
#include <QShowEvent>
#include <QKeyEvent>

//предварительные объявления всех классов
class Scene;
class BasePrimitive;
class BaseCreationTool;
class BaseDrawingTool;
class DeleteTool;
class SegmentCreationTool;
class PointPrimitive;

class ViewportPanelWidget;
class ToolbarPanelWidget;
class PropertiesPanelWidget;
class SceneObjectsPanelWidget;
class SceneSettingsPanelWidget;
class ConsolePanelWidget;
class CommandParser;

//QMainWindow - базовый шаблон Qt для создания главного окна
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    //конструктор и деструктор
    MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

private slots:
    //слоты активации инструмента
    void activateDeleteTool();
    void activateSegmentCreationTool();

    //слоты создания новых объектов
    void createNewSegment(const PointPrimitive& start, const PointPrimitive& end);

    //слот удаления объекта
    void deletePrimitive(BasePrimitive* primitive);

    //слот вызова окна настроек
    void openSettingsDialog();

    //слот отключения выбранного инструмента
    void deactivateCurrentTool();

    void processConsoleCommand(const QString& command);

signals:
    //сигнал, который сообщает, что сцена изменилась
    //SceneObjectsPanelWidget слушает этот сигнал, чтобы обновить свой список
    void sceneChanged(const Scene* scene);

    //сигнал, который сообщает, что объект выбран
    //PropertiesPanelWidget слушает этот сигнал, чтобы показать свойства этого объекта
    void objectSelected(BasePrimitive* primitive);

    //сигнал, который сообщает, что инструмент был активирован
    //PropertiesPanelWidget слушает этот сигнал, чтобы показать пустую форму для создания нового объекта
    void toolActivated(PrimitiveType type);

protected:
    //переопределение метода перехватки событий клавиатуры
    void keyPressEvent(QKeyEvent* event) override;

private:
    void createTools(); //метод создания инструментов
    void createDrawingTools(); //метод создание отрисовщиков
    void createPanelWindows(); //метод создания интерфейсных панелей
    void createConnections(); //метод создания взаимодействий

    void addPrimitiveToScene(BasePrimitive* primitive); //метод добавления примитива в сцену

    void updateApplicationIcons(); //метод обновления всех иконок
    void createActions(); //метод для создания QAction
    void createMenus(); //метод для создания меню

    void showEvent(QShowEvent* event) override; //переопределение метода создания первичного окна приложения

    Scene* m_scene; //указатель на объект сцены
    BaseCreationTool* m_currentTool; //указатель на выбранный инструмент
    std::map<PrimitiveType, std::unique_ptr<BaseDrawingTool>> m_drawingTools; //контейнер отрисовщиков

    bool m_isInitialResizeDone = false; //флаг для однократного выполнения кода в showEvent

    BasePrimitive* m_selectedPrimitive = nullptr;

    CommandParser* m_commandParser;

    //инструменты
    DeleteTool* m_deleteTool;
    SegmentCreationTool* m_segmentCreationTool;

    //интерфейсные панели
    ViewportPanelWidget* m_viewportPanel;
    ToolbarPanelWidget* m_toolbarPanel;
    PropertiesPanelWidget* m_propertiesPanel;
    SceneObjectsPanelWidget* m_sceneObjectsPanel;
    SceneSettingsPanelWidget* m_sceneSettingsPanel;
    ConsolePanelWidget* m_consolePanel;

    //окно настроек
    QAction* m_settingsAction;
};
