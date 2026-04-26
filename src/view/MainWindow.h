//MainWindow - класс, который владеет, компонует и связвыет остальные классы между собой

#pragma once

#include "BasePrimitive.h"
#include "../model/primitives/dimensions/LinearDimensionPrimitive.h"
#include <QMainWindow>
#include <QShowEvent>
#include <QKeyEvent>
#include <QList>

//предварительные объявления всех классов
class Scene;
class PointPrimitive;
class SegmentPrimitive;
class CirclePrimitive;
class EllipsePrimitive;
class RectanglePrimitive;
class ArcPrimitive;
class PolygonPrimitive;
class SplinePrimitive;
class LinearDimensionPrimitive;
class RadialDimensionPrimitive;
class AngularDimensionPrimitive;

class BaseCreationTool;
class BaseDrawingTool;
class DeleteTool;
class MoveTool;
class SegmentCreationTool;
class CircleCreationTool;
class RectangleCreationTool;
class ArcCreationTool;
class EllipseCreationTool;
class PolygonCreationTool;
class SplineCreationTool;
class LinearDimensionCreationTool;
class RadialDimensionCreationTool;
class AngularDimensionCreationTool;

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
    void activateRectangleTool(RectangleCreationMode mode = RectangleCreationMode::TwoPoints);
    void activateArcTool(ArcCreationMode mode = ArcCreationMode::ThreePoints);
    void activateEllipseTool();
    void activatePolygonTool();
    void activateSplineTool();
    void activateLinearDimensionTool();
    void activateLinearDimensionTool(LinearDimensionMode mode);
    void activateDimensionTool(DimensionCreationMode mode);
    void activateRadialDimensionTool(bool isDiameter);
    void activateAngularDimensionTool();

    //слот установки цвета для инструмента
    void onColorChanged(const QColor& color);

    //слот установки типа линии для инструмента
    void onLineTypeChanged(LineType type);

    //слот установки слоя для выбранных объектов и последующих
    void onLayerChanged(const QString& name);

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
    void applyRectangleChanges(RectanglePrimitive* rect, const PointPrimitive& center, double w, double h, double r, CornerType cornerType, double cornerRadius, const QColor& c, LineType t);
    void applyArcChanges(ArcPrimitive* arc, const PointPrimitive& center, double rad, double start, double span, const QColor& c, LineType t);
    void applyEllipseChanges(EllipsePrimitive* ell, const PointPrimitive& center, double rx, double ry, double rot, const QColor& c, LineType t);
    void applyPolygonChanges(PolygonPrimitive* polygon, int sides, PolygonCreationMode type, const QColor& color, LineType lineType);
    void applySplineChanges(SplinePrimitive* spline, bool closed, const QVector<QPointF>& controlPoints, const QColor& c, LineType t);

    //слот применения общих свойств ко всем выделенным объектам
    void applyCommonProperties(const QColor& color, int lineTypeId);

    //слот удаления объекта
    void deletePrimitive(BasePrimitive* primitive);

    //слот вызова окна настроек
    void openSettingsWindow();

    //слот экспорта в DXF
    void onExportDxf();

    //слот импорта из DXF
    void onImportDxf();

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
    void refreshAssociativeDimensions();
    void applyGlobalDimensionStyleToScene();

    void showEvent(QShowEvent* event) override; //переопределение метода создания первичного окна приложения

    Scene* m_scene; //указатель на объект сцены
    BaseCreationTool* m_currentTool; //указатель на выбранный инструмент
    bool m_isInitialResizeDone = false; //флаг для однократного выполнения кода в showEvent
    QList<BasePrimitive*> m_selectedPrimitives; //указатель на выбранные объекта
    QCursor m_previousCursor; //позиция курсора
    QString m_currentLayer = "0"; //текущий слой

    //инструменты
    DeleteTool* m_deleteTool;
    MoveTool* m_moveTool;
    SegmentCreationTool* m_segmentCreationTool;
    CircleCreationTool* m_circleCreationTool;
    RectangleCreationTool* m_rectCreationTool;
    ArcCreationTool* m_arcCreationTool;
    EllipseCreationTool* m_ellipseCreationTool;
    PolygonCreationTool* m_polygonCreationTool;
    SplineCreationTool* m_splineCreationTool;
    LinearDimensionCreationTool* m_linearDimCreationTool;
    RadialDimensionCreationTool* m_radialDimCreationTool;
    AngularDimensionCreationTool* m_angularDimCreationTool;

    //интерфейсные панели
    ViewportPanelWidget* m_viewportPanel;
    NavigationPanelWidget* m_navigationPanel;
    ToolbarPanelWidget* m_toolbarPanel;
    PropertiesPanelWidget* m_propertiesPanel;
    SceneObjectsPanelWidget* m_sceneObjectsPanel;
    SceneSettingsPanelWidget* m_sceneSettingsPanel;
    ConsolePanelWidget* m_consolePanel;

    //окно настроек и экспорт
    QAction* m_settingsAction;
    QAction* m_exportDxfAction;
    QAction* m_importDxfAction;
};
