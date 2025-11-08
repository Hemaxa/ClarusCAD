//MainWindow - класс, который владеет, компонует и связвыет остальные классы между собой

#pragma once

#include "BasePrimitive.h"
#include "PointPrimitive.h"

#include <QMainWindow>
#include <QShowEvent>
#include <QKeyEvent>

//предварительные объявления всех классов
class Scene;
class PointPrimitive;
class SegmentPrimitive;
class BaseCreationTool;
class BaseDrawingTool;
class DeleteTool;
class MoveTool;
class SegmentCreationTool;

class ConsolePanelWidget;
class PropertiesPanelWidget;
class SceneObjectsPanelWidget;
class SceneSettingsPanelWidget;
class ToolbarPanelWidget;
class ViewportPanelWidget;

struct ParsedCommand;

//QMainWindow - базовый шаблон Qt для создания главного окна
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    //конструктор и деструктор
    MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

//то, что улавливается
private slots:
    //слоты активации инструментов
    void activateDeleteTool();
    void activateMoveTool();
    void activateSegmentCreationTool();

    //слот установки цвета для инструмента
    void onColorChanged(const QColor& color);

    //слот установки типа линии для инструмента
    void onLineTypeChanged(LineType type);

    //слот установки выбранных объектов
    void onSelectionChanged(BasePrimitive* primitive);

    //слот обработки команды из консоли
    void onConsoleCommandParsed(const ParsedCommand& command);

    //слот отключения выбранного инструмента
    void deactivateCurrentTool();

    //слоты создания или обновления объектов
    void applySegmentChanges(SegmentPrimitive* segment, const PointPrimitive& start, const PointPrimitive& end, const QColor& color, LineType lineType);

    //слот удаления объекта
    void deletePrimitive(BasePrimitive* primitive);

    //слот вызова окна настроек
    void openSettingsWindow();

//то, что посылается
signals:
    //сигнал, который сообщает, что сцена изменилась
    //SceneObjectsPanelWidget слушает этот сигнал, чтобы обновить свой список
    void sceneChanged(const Scene* scene);

    //сигнал, который сообщает, что инструмент был активирован
    //PropertiesPanelWidget слушает этот сигнал, чтобы показать пустую форму для создания нового объекта
    void toolActivated(PrimitiveType type);

protected:
    //переопределение методов перехватки событий клавиатуры
    void keyPressEvent(QKeyEvent* event) override;
    void keyReleaseEvent(QKeyEvent* event) override;

private:
    void createTools(); //метод создания инструментов
    void createPanelWindows(); //метод создания интерфейсных панелей
    void createConnections(); //метод создания взаимодействий
    void createMenus(); //метод для создания меню
    void createActions(); //метод для создания QAction

    void addPrimitiveToScene(BasePrimitive* primitive); //метод добавления примитива в сцену

    void showEvent(QShowEvent* event) override; //переопределение метода создания первичного окна приложения

    Scene* m_scene; //указатель на объект сцены
    BaseCreationTool* m_currentTool; //указатель на выбранный инструмент

    bool m_isInitialResizeDone = false; //флаг для однократного выполнения кода в showEvent

    BasePrimitive* m_selectedPrimitive = nullptr; //указатель на выбранный объект

    QCursor m_previousCursor; //позиция курсора

    //инструменты
    DeleteTool* m_deleteTool;
    MoveTool* m_moveTool;
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
