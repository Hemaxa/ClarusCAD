//ViewportPanelWidget - панель окна просмотра сцены

#pragma once

#include "BasePanelWidget.h"
#include "BasePrimitive.h"
#include "EnumManager.h"

#include <QPointF>
#include <QList>
#include <map>
#include <memory>
#include "SnapManager.h"

class Scene;
class ViewportCamera;
class ThemeManager;
class BaseCreationTool;
class BaseDrawingTool;
class QLabel;

//наслдедуется от базового класса BasePanelWidget
class ViewportPanelWidget : public BasePanelWidget
{
    Q_OBJECT

public:
    //конструктор и деструктор
    explicit ViewportPanelWidget(const QString& title, QWidget* parent = nullptr);
    ~ViewportPanelWidget();

    //этими методами MainWindow настраивает и управляет вьюпортом
    void setScene(Scene* scene);
    void setActiveTool(BaseCreationTool* tool);
    void setGridStep(int step);
    void setSelectedPrimitives(const QList<BasePrimitive*>& primitives);
    QList<BasePrimitive*> getSelectedPrimitives() const;

    int getGridStep() const;
    double getDynamicGridStep() const;
    double getZoomFactor() const;
    QWidget* getCanvas() const;

    //Геттер для точки привязки (теперь просто делегирует в ObjectBindingManager)
    QPointF getSnappedPoint(const QPointF& worldPos) const;

    //методы для трансформации координат
    QPointF worldToScreen(const QPointF& worldPos) const;
    QPointF screenToWorld(const QPointF& screenPos) const;

    void update();

public slots:
    void applyZoom(double factor, const QPoint& anchorPoint);
    void rotateSceneLeft();
    void rotateSceneRight();

    void zoomIn();
    void zoomIn(const QPoint& anchorPoint);
    void zoomOut();
    void zoomOut(const QPoint& anchorPoint);
    void zoomToExtents();

    void setZoomStep(double step);
    void setCoordinateSystem(CoordinateSystemType type);
    void setGridSnapEnabled(bool enabled);
    void setPrimitiveSnapEnabled(bool enabled);
    void setSelectedPrimitive(BasePrimitive* primitive);

    void panWorld(const QPointF& worldDelta);

signals:
    void mouseMoved(const QPoint& screenPos);
    void selectionChanged(const QList<BasePrimitive*>& primitives);

private slots:
    void onCameraUpdated();

private:
    Scene* m_scene = nullptr;
    BasePrimitive* m_selectedPrimitive = nullptr;
    QLabel* m_infoLabel;
    BaseCreationTool* m_activeTool = nullptr;

    std::map<PrimitiveType, std::unique_ptr<BaseDrawingTool>> m_drawingTools;

    ThemeManager* m_themeManager = nullptr;

    //параметры панели по умолчанию
    int m_gridStep = 50;
    double m_zoomStep = 1.25;
    QPoint m_lastPanPos;

    bool m_isPanning = false;
    bool m_isGridSnapEnabled = true;
    bool m_isPrimitiveSnapEnabled = true;

    QPointF m_currentMouseWorldPos;
    double m_gridMultiplier = 1.0;
    CoordinateSystemType m_coordSystemType = CoordinateSystemType::Cartesian;

    ViewportCamera* m_camera;

    QRect getGizmoRect() const;

    bool eventFilter(QObject* obj, QEvent* event) override;
    void paintCanvas(QPaintEvent* event);
    void paintGrid(QPainter& painter, const QTransform& worldTransform);
    void paintGizmo(QPainter& painter);
    double calculateDynamicGridStep() const;
    void createDrawingTools();
    QPointF snapToGrid(const QPointF& worldPos) const;
    QPointF snapToPrimitives(const QPointF& worldPos) const;
    void updateInfoLabel();

    QList<BasePrimitive*> m_selectedPrimitives;

    // ПОЛЯ ДЛЯ РАМКИ
    bool m_isSelecting = false;
    QPoint m_selectionStartPos;
    QPoint m_currentMousePosScreen;
    
    mutable SnapPoint m_lastSnapPoint; // Текущая точка привязки для визуализации
};
