#include "ToolbarPanelWidget.h"
#include "AnimationManager.h"

#include <QButtonGroup>
#include <QToolButton>
#include <QGridLayout>
#include <QMenu>
#include <QAction>

ToolbarPanelWidget::ToolbarPanelWidget(const QString& title, QWidget* parent) : BasePanelWidget(title, parent)
{
    //сетка для всех кнопок
    auto* layout = new QGridLayout(canvas());
    layout->setContentsMargins(10, 10, 10, 10);

    //общая группа для кнопок на панели инструментов (для возможности выбора только одной)
    m_buttonGroup = new QButtonGroup(this);
    m_buttonGroup->setExclusive(true);

    //создание кнопок
    m_deleteBtn = new AnimationManager(":/icons/icons/tools/delete.svg", "Удаление [X]", Qt::Key_X, true);
    m_moveBtn = new AnimationManager(":/icons/icons/tools/move.svg", "Перемещение [M]", Qt::Key_M, true);
    m_createSegmentBtn = new AnimationManager(":/icons/icons/tools/segment.svg", "Отрезок [S]", Qt::Key_S, true);
    m_createCircleBtn = new AnimationManager(":/icons/icons/tools/move.svg", "Окружность [C]", Qt::Key_C, true);
    m_createRectBtn = new AnimationManager(":/icons/icons/tools/rectangle.svg", "Прямоугольник [R]", Qt::Key_R, true);
    m_createArcBtn = new AnimationManager(":/icons/icons/tools/arc.svg", "Дуга [A]", Qt::Key_A, true);
    m_createEllipseBtn = new AnimationManager(":/icons/icons/tools/ellipse.svg", "Эллипс [E]", Qt::Key_E, true);

    // === POPUP МЕНЮ ДЛЯ ОКРУЖНОСТИ ===
    m_createCircleBtn->setPopupMode(QToolButton::MenuButtonPopup);
    QMenu* circleMenu = new QMenu(m_createCircleBtn);
    QAction* actCircleCenterRadius = circleMenu->addAction("Центр, Радиус");
    QAction* actCircleCenterDiameter = circleMenu->addAction("Центр, Диаметр");
    QAction* actCircle2Points = circleMenu->addAction("Две точки");
    QAction* actCircle3Points = circleMenu->addAction("Три точки");
    m_createCircleBtn->setMenu(circleMenu);

    // === POPUP МЕНЮ ДЛЯ ПРЯМОУГОЛЬНИКА ===
    m_createRectBtn->setPopupMode(QToolButton::MenuButtonPopup);
    QMenu* rectMenu = new QMenu(m_createRectBtn);
    QAction* actRect2Points = rectMenu->addAction("Две точки");
    QAction* actRectCenterSize = rectMenu->addAction("Центр и размер");
    QAction* actRect3Points = rectMenu->addAction("Три точки");
    m_createRectBtn->setMenu(rectMenu);

    // === POPUP МЕНЮ ДЛЯ ДУГИ ===
    m_createArcBtn->setPopupMode(QToolButton::MenuButtonPopup);
    QMenu* arcMenu = new QMenu(m_createArcBtn);
    QAction* actArc3Points = arcMenu->addAction("Три точки");
    QAction* actArcCenterStartEnd = arcMenu->addAction("Центр, Начало, Конец");
    m_createArcBtn->setMenu(arcMenu);

    //добавление кнопок в общую группу
    m_buttonGroup->addButton(m_deleteBtn);
    m_buttonGroup->addButton(m_moveBtn);
    m_buttonGroup->addButton(m_createSegmentBtn);
    m_buttonGroup->addButton(m_createCircleBtn);
    m_buttonGroup->addButton(m_createRectBtn);
    m_buttonGroup->addButton(m_createArcBtn);
    m_buttonGroup->addButton(m_createEllipseBtn);

    //добавление кнопок в layout
    layout->addWidget(m_deleteBtn, 0, 0, Qt::AlignLeft);
    layout->addWidget(m_moveBtn, 1, 0, Qt::AlignLeft);
    layout->addWidget(m_createSegmentBtn, 2, 0, Qt::AlignLeft);
    layout->addWidget(m_createCircleBtn, 3, 0, Qt::AlignLeft);
    layout->addWidget(m_createRectBtn, 4, 0, Qt::AlignLeft);
    layout->addWidget(m_createArcBtn, 5, 0, Qt::AlignLeft);
    layout->addWidget(m_createEllipseBtn, 6, 0, Qt::AlignLeft);

    layout->setColumnStretch(1, 1);
    layout->setRowStretch(7, 1);

    //подключение сигналов
    connect(m_deleteBtn, &QToolButton::clicked, this, &ToolbarPanelWidget::deleteToolActivated);
    connect(m_moveBtn, &QToolButton::clicked, this, &ToolbarPanelWidget::moveToolActivated);
    connect(m_createSegmentBtn, &QToolButton::clicked, this, &ToolbarPanelWidget::segmentToolActivated);
    connect(m_createEllipseBtn, &QToolButton::clicked, this, &ToolbarPanelWidget::ellipseToolActivated);

    // Circle: клик по кнопке = CenterRadius
    connect(m_createCircleBtn, &QToolButton::clicked, this, [this]() {
        emit circleToolActivated(CircleCreationMode::CenterRadius);
    });
    connect(actCircleCenterRadius, &QAction::triggered, this, [this]() {
        m_createCircleBtn->click();
        emit circleToolActivated(CircleCreationMode::CenterRadius);
    });
    connect(actCircleCenterDiameter, &QAction::triggered, this, [this]() {
        m_createCircleBtn->click();
        emit circleToolActivated(CircleCreationMode::CenterDiameter);
    });
    connect(actCircle2Points, &QAction::triggered, this, [this]() {
        m_createCircleBtn->click();
        emit circleToolActivated(CircleCreationMode::TwoPoints);
    });
    connect(actCircle3Points, &QAction::triggered, this, [this]() {
        m_createCircleBtn->click();
        emit circleToolActivated(CircleCreationMode::ThreePoints);
    });

    // Rectangle: клик = TwoPoints
    connect(m_createRectBtn, &QToolButton::clicked, this, [this]() {
        emit rectangleToolActivated(RectangleCreationMode::TwoPoints);
    });
    connect(actRect2Points, &QAction::triggered, this, [this]() {
        m_createRectBtn->click();
        emit rectangleToolActivated(RectangleCreationMode::TwoPoints);
    });
    connect(actRectCenterSize, &QAction::triggered, this, [this]() {
        m_createRectBtn->click();
        emit rectangleToolActivated(RectangleCreationMode::CenterSize);
    });
    connect(actRect3Points, &QAction::triggered, this, [this]() {
        m_createRectBtn->click();
        emit rectangleToolActivated(RectangleCreationMode::ThreePoints);
    });

    // Arc: клик = ThreePoints
    connect(m_createArcBtn, &QToolButton::clicked, this, [this]() {
        emit arcToolActivated(ArcCreationMode::ThreePoints);
    });
    connect(actArc3Points, &QAction::triggered, this, [this]() {
        m_createArcBtn->click();
        emit arcToolActivated(ArcCreationMode::ThreePoints);
    });
    connect(actArcCenterStartEnd, &QAction::triggered, this, [this]() {
        m_createArcBtn->click();
        emit arcToolActivated(ArcCreationMode::CenterStartEnd);
    });

    setMinimumWidth(200);
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
