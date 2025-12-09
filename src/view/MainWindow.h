//MainWindow - класс, который владеет, компонует и связвыет остальные классы между собой

#pragma once

#include "BasePrimitive.h"
#include "PointPrimitive.h"
#include "RectangleCreationTool.h"
#include "ArcCreationTool.h"
#include "RectanglePrimitive.h"
#include "ArcPrimitive.h"

#include <QMainWindow>
#include <QShowEvent>
#include <QKeyEvent>
#include <QList>

//предварительные объявления всех классов
class Scene;
class PointPrimitive;
class SegmentPrimitive;
class CirclePrimitive;

class BaseCreationTool;
class BaseDrawingTool;
class DeleteTool;
class MoveTool;
class SegmentCreationTool;
class CircleCreationTool;

class ConsolePanelWidget;
class NavigationPanelWidget;
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
    void activateCircleCreationTool(CircleCreationMode mode);
    void activateRectangleTool();
    void activateArcTool();

    //слот установки цвета для инструмента
    void onColorChanged(const QColor& color);

    //слот установки типа линии для инструмента
    void onLineTypeChanged(LineType type);

    void onRotateLeft();

    void onRotateRight();

    void onZoomExtents();

    //слот установки выбранных объектов
    void onSelectionChanged(const QList<BasePrimitive*>& primitives);

    //слот обработки команды из консоли
    void onConsoleCommandParsed(const ParsedCommand& command);

    //слот отключения выбранного инструмента
    void deactivateCurrentTool();

    //слоты создания или обновления объектов
    void applySegmentChanges(SegmentPrimitive* segment, const PointPrimitive& start, const PointPrimitive& end, const QColor& color, LineType lineType);
    void applyCircleChanges(CirclePrimitive* circle, const PointPrimitive& center, double radius, const QColor& color, LineType lineType); // <---
    void applyRectangleChanges(RectanglePrimitive* rect, const PointPrimitive& center, double w, double h, double r, const QColor& c, LineType t);
    void applyArcChanges(ArcPrimitive* arc, const PointPrimitive& center, double rad, double start, double span, const QColor& c, LineType t);


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
    QList<BasePrimitive*> m_selectedPrimitives; //указатель на выбранные объекта
    QCursor m_previousCursor; //позиция курсора

    //инструменты
    DeleteTool* m_deleteTool;
    MoveTool* m_moveTool;
    SegmentCreationTool* m_segmentCreationTool;
    CircleCreationTool* m_circleCreationTool;
    RectangleCreationTool* m_rectCreationTool;
    ArcCreationTool* m_arcCreationTool;

    //интерфейсные панели
    ViewportPanelWidget* m_viewportPanel;
    NavigationPanelWidget* m_navigationPanel;
    ToolbarPanelWidget* m_toolbarPanel;
    PropertiesPanelWidget* m_propertiesPanel;
    SceneObjectsPanelWidget* m_sceneObjectsPanel;
    SceneSettingsPanelWidget* m_sceneSettingsPanel;
    ConsolePanelWidget* m_consolePanel;

    //окно настроек
    QAction* m_settingsAction;
};
