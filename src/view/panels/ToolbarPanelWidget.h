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
    /**
     * @brief Конструктор панели инструментов.
     */
    explicit ToolbarPanelWidget(const QString& title, QWidget* parent = nullptr);

    /**
     * @brief Снять визуальное выделение со всех инструментов.
     */
    void clearSelection();

    // Геттеры для доступа к кнопкам (например, для настройки горячих клавиш)
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
    // Сигналы активации инструментов
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
    QButtonGroup* m_buttonGroup; ///< Группа кнопок для эксклюзивного выбора

    // Указатели на кнопки
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
