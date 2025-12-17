//ToolbarPanelWidget - панель инструментов

#pragma once

#include "EnumManager.h"
#include "BasePanelWidget.h"

class QToolButton;
class QButtonGroup;
class FlyoutToolButton;

//наслдедуется от базового класса BasePanelWidget
class ToolbarPanelWidget : public BasePanelWidget
{
    Q_OBJECT

public:
    //конструктор
    explicit ToolbarPanelWidget(const QString& title, QWidget* parent = nullptr);

    //метод снятия выделения с инструментов
    void clearSelection();

    //геттры для указателей на кнопки
    QToolButton* getDeleteButton() const;
    QToolButton* getMoveButton() const;
    QToolButton* getCreateSegmentButton() const;
    QToolButton* getCreateCircleButton() const;
    QToolButton* getCreateRectangleButton() const;
    QToolButton* getCreateArcButton() const;
    QToolButton* getCreateEllipseButton() const;
    QToolButton* getCreatePolygonButton() const;
    QToolButton* getCreateSplineButton() const;

signals:
    //сигналы нажатия соответствующих кнопок
    void deleteToolActivated();
    void moveToolActivated();
    void segmentToolActivated();
    void circleToolActivated(CircleCreationMode mode);
    void rectangleToolActivated(RectangleCreationMode mode);
    void arcToolActivated(ArcCreationMode mode);
    void ellipseToolActivated();
    void polygonToolActivated();
    void splineToolActivated();

private:
    //группа для кнопок на панели инструментов
    QButtonGroup* m_buttonGroup;

    //указатели на соответствующие кнопки
    QToolButton* m_deleteBtn;
    QToolButton* m_moveBtn;
    QToolButton* m_createSegmentBtn;
    FlyoutToolButton* m_createCircleBtn;
    FlyoutToolButton* m_createRectBtn;
    FlyoutToolButton* m_createArcBtn;
    QToolButton* m_createEllipseBtn;
    QToolButton* m_createPolygonBtn;
    QToolButton* m_createSplineBtn;
};
