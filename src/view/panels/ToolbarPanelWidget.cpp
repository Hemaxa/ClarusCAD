#include "ToolbarPanelWidget.h"
#include "AnimationManager.h"
#include "FlyoutToolButton.h"

#include <QButtonGroup>
#include <QToolButton>
#include <QGridLayout>

ToolbarPanelWidget::ToolbarPanelWidget(const QString& title, QWidget* parent) : BasePanelWidget(title, parent)
{
    //сетка для всех кнопок
    auto* layout = new QGridLayout(canvas());
    layout->setContentsMargins(10, 10, 10, 10);

    //общая группа для кнопок на панели инструментов (для возможности выбора только одной)
    m_buttonGroup = new QButtonGroup(this);
    m_buttonGroup->setExclusive(true);

    //создание простых кнопок (без режимов)
    m_deleteBtn = new AnimationManager(":/icons/icons/tools/delete.svg", "Удаление [X]", Qt::Key_X, true);
    m_moveBtn = new AnimationManager(":/icons/icons/tools/move.svg", "Перемещение [M]", Qt::Key_M, true);
    m_createSegmentBtn = new AnimationManager(":/icons/icons/tools/segment.svg", "Отрезок [S]", Qt::Key_S, true);
    m_createEllipseBtn = new AnimationManager(":/icons/icons/tools/ellipse.svg", "Эллипс [E]", Qt::Key_E, true);
    m_createPolygonBtn = new AnimationManager(":/icons/icons/tools/polygon.svg", "Многоугольник [P]", Qt::Key_P, true);
    m_createSplineBtn = new AnimationManager(":/icons/icons/tools/spline.svg", "Сплайн [L]", Qt::Key_L, true);

    // === FLYOUT КНОПКА ДЛЯ ОКРУЖНОСТИ ===
    m_createCircleBtn = new FlyoutToolButton(this);
    m_createCircleBtn->addMode(static_cast<int>(CircleCreationMode::CenterRadius), ":/icons/icons/tools/circle_center_radius.svg", "Центр, Радиус [C]");
    m_createCircleBtn->addMode(static_cast<int>(CircleCreationMode::CenterDiameter), ":/icons/icons/tools/circle_center_diameter.svg", "Центр, Диаметр");
    m_createCircleBtn->addMode(static_cast<int>(CircleCreationMode::TwoPoints), ":/icons/icons/tools/circle_2points.svg", "Две точки");
    m_createCircleBtn->addMode(static_cast<int>(CircleCreationMode::ThreePoints), ":/icons/icons/tools/circle_3points.svg", "Три точки");
    m_createCircleBtn->setFixedSize(40, 40);
    m_createCircleBtn->setIconSize(QSize(28, 28));

    // === FLYOUT КНОПКА ДЛЯ ПРЯМОУГОЛЬНИКА ===
    m_createRectBtn = new FlyoutToolButton(this);
    m_createRectBtn->addMode(static_cast<int>(RectangleCreationMode::TwoPoints), ":/icons/icons/tools/rect_2points.svg", "Две точки [R]");
    m_createRectBtn->addMode(static_cast<int>(RectangleCreationMode::CenterSize), ":/icons/icons/tools/rect_center.svg", "Центр и размер");
    m_createRectBtn->addMode(static_cast<int>(RectangleCreationMode::PointSize), ":/icons/icons/tools/rect_3points.svg", "Точка и размер");
    m_createRectBtn->setFixedSize(40, 40);
    m_createRectBtn->setIconSize(QSize(28, 28));

    // === FLYOUT КНОПКА ДЛЯ ДУГИ ===
    m_createArcBtn = new FlyoutToolButton(this);
    m_createArcBtn->addMode(static_cast<int>(ArcCreationMode::ThreePoints), ":/icons/icons/tools/arc_3points.svg", "Три точки [A]");
    m_createArcBtn->addMode(static_cast<int>(ArcCreationMode::CenterStartEnd), ":/icons/icons/tools/arc_center.svg", "Центр, Начало, Конец");
    m_createArcBtn->setFixedSize(40, 40);
    m_createArcBtn->setIconSize(QSize(28, 28));

    //добавление кнопок в общую группу
    m_buttonGroup->addButton(m_deleteBtn);
    m_buttonGroup->addButton(m_moveBtn);
    m_buttonGroup->addButton(m_createSegmentBtn);
    m_buttonGroup->addButton(m_createCircleBtn);
    m_buttonGroup->addButton(m_createRectBtn);
    m_buttonGroup->addButton(m_createArcBtn);
    m_buttonGroup->addButton(m_createEllipseBtn);
    m_buttonGroup->addButton(m_createPolygonBtn);
    m_buttonGroup->addButton(m_createSplineBtn);

    //добавление кнопок в layout
    layout->addWidget(m_deleteBtn, 0, 0, Qt::AlignLeft);
    layout->addWidget(m_moveBtn, 1, 0, Qt::AlignLeft);
    layout->addWidget(m_createSegmentBtn, 2, 0, Qt::AlignLeft);
    layout->addWidget(m_createCircleBtn, 3, 0, Qt::AlignLeft);
    layout->addWidget(m_createRectBtn, 4, 0, Qt::AlignLeft);
    layout->addWidget(m_createArcBtn, 5, 0, Qt::AlignLeft);
    layout->addWidget(m_createEllipseBtn, 6, 0, Qt::AlignLeft);
    layout->addWidget(m_createPolygonBtn, 7, 0, Qt::AlignLeft);
    layout->addWidget(m_createSplineBtn, 8, 0, Qt::AlignLeft);

    layout->setColumnStretch(1, 1);
    layout->setRowStretch(9, 1);

    //подключение сигналов для простых кнопок
    connect(m_deleteBtn, &QToolButton::clicked, this, &ToolbarPanelWidget::deleteToolActivated);
    connect(m_moveBtn, &QToolButton::clicked, this, &ToolbarPanelWidget::moveToolActivated);
    connect(m_createSegmentBtn, &QToolButton::clicked, this, &ToolbarPanelWidget::segmentToolActivated);
    connect(m_createEllipseBtn, &QToolButton::clicked, this, &ToolbarPanelWidget::ellipseToolActivated);
    connect(m_createPolygonBtn, &QToolButton::clicked, this, &ToolbarPanelWidget::polygonToolActivated);
    connect(m_createSplineBtn, &QToolButton::clicked, this, &ToolbarPanelWidget::splineToolActivated);

    // === ПОДКЛЮЧЕНИЕ FLYOUT КНОПОК ===
    
    // Circle: modeActivated передает int, который можно напрямую преобразовать в CircleCreationMode
    connect(m_createCircleBtn, &FlyoutToolButton::modeActivated, this, [this](int modeId) {
        emit circleToolActivated(static_cast<CircleCreationMode>(modeId));
    });

    // Rectangle: modeActivated -> RectangleCreationMode
    connect(m_createRectBtn, &FlyoutToolButton::modeActivated, this, [this](int modeId) {
        emit rectangleToolActivated(static_cast<RectangleCreationMode>(modeId));
    });

    // Arc: modeActivated -> ArcCreationMode
    connect(m_createArcBtn, &FlyoutToolButton::modeActivated, this, [this](int modeId) {
        emit arcToolActivated(static_cast<ArcCreationMode>(modeId));
    });

    setMinimumWidth(160);
}

void ToolbarPanelWidget::clearSelection()
{
    //получение указателя на активную кнопку
    if (m_buttonGroup->checkedButton()) {
        //отключение кнопки
        m_buttonGroup->setExclusive(false);
        m_buttonGroup->checkedButton()->setChecked(false);
        m_buttonGroup->setExclusive(true);
    }
}

QToolButton* ToolbarPanelWidget::getDeleteButton() const { return m_deleteBtn; }
QToolButton* ToolbarPanelWidget::getMoveButton() const { return m_moveBtn; }
QToolButton* ToolbarPanelWidget::getCreateSegmentButton() const { return m_createSegmentBtn; }
QToolButton* ToolbarPanelWidget::getCreateCircleButton() const { return m_createCircleBtn; }
QToolButton* ToolbarPanelWidget::getCreateRectangleButton() const { return m_createRectBtn; }
QToolButton* ToolbarPanelWidget::getCreateArcButton() const { return m_createArcBtn; }
QToolButton* ToolbarPanelWidget::getCreateEllipseButton() const { return m_createEllipseBtn; }
QToolButton* ToolbarPanelWidget::getCreatePolygonButton() const { return m_createPolygonBtn; }
QToolButton* ToolbarPanelWidget::getCreateSplineButton() const { return m_createSplineBtn; }
