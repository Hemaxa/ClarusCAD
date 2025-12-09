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
    m_buttonGroup = new QButtonGroup(this); //создание группы кнопок (this - указатель на родителя для автоматического контроля памяти)
    m_buttonGroup->setExclusive(true); //возможность выбора только одной кнопки

    //создание и добавление кнопок в группу
    //шаблон: путь до иконки, текст описания, горячая клавиша, залипание
    m_deleteBtn = new AnimationManager(":/icons/icons/tools/delete.svg", "Удаление [X]", Qt::Key_X, true);
    m_moveBtn = new AnimationManager(":/icons/icons/tools/move.svg", "Перемещение [M]", Qt::Key_M, true);
    m_createSegmentBtn = new AnimationManager(":/icons/icons/tools/segment.svg", "Отрезок [S]", Qt::Key_S, true);
    m_createCircleBtn = new AnimationManager(":/icons/icons/tools/circle.svg", "Окружность [C]", Qt::Key_C, true);
    m_createRectBtn = new AnimationManager(":/icons/icons/tools/rectangle.svg", "Прямоугольник [R]", Qt::Key_R, true);
    m_createArcBtn = new AnimationManager(":/icons/icons/tools/arc.svg", "Дуга [A]", Qt::Key_A, true);

    //настройка выпадающих меню
    //кнопка построения окружности
    m_createCircleBtn->setPopupMode(QToolButton::MenuButtonPopup);
    QMenu* circleMenu = new QMenu(m_createCircleBtn);
    QAction* actCenterRadius = circleMenu->addAction("Центр, Радиус");
    QAction* actCenterDiameter = circleMenu->addAction("Центр, Диаметр");
    QAction* act2Points = circleMenu->addAction("Две точки");
    QAction* act3Points = circleMenu->addAction("Три точки");
    m_createCircleBtn->setMenu(circleMenu);

    //добавление кнопок в общую группу
    m_buttonGroup->addButton(m_deleteBtn);
    m_buttonGroup->addButton(m_moveBtn);
    m_buttonGroup->addButton(m_createSegmentBtn);
    m_buttonGroup->addButton(m_createCircleBtn);
    m_buttonGroup->addButton(m_createRectBtn);
    m_buttonGroup->addButton(m_createArcBtn);

    //добавление кнопок в шаблон
    layout->addWidget(m_deleteBtn, 0, 0, Qt::AlignLeft);
    layout->addWidget(m_moveBtn, 1, 0, Qt::AlignLeft);
    layout->addWidget(m_createSegmentBtn, 2, 0, Qt::AlignLeft);
    layout->addWidget(m_createCircleBtn, 3, 0, Qt::AlignLeft);
    layout->addWidget(m_createRectBtn, 4, 0, Qt::AlignLeft);
    layout->addWidget(m_createArcBtn, 5, 0, Qt::AlignLeft);

    //последняя пустая колонка должна растягиваться, прижимая кнопки влево
    layout->setColumnStretch(1, 1);

    //последняя путсая строка должна растягиваться, прижимая кнопки вверх
    layout->setRowStretch(6, 1);

    //подключение сигналов от кнопок
    connect(m_deleteBtn, &QToolButton::clicked, this, &ToolbarPanelWidget::deleteToolActivated);
    connect(m_moveBtn, &QToolButton::clicked, this, &ToolbarPanelWidget::moveToolActivated);
    connect(m_createSegmentBtn, &QToolButton::clicked, this, &ToolbarPanelWidget::segmentToolActivated);
    connect(m_createRectBtn, &QToolButton::clicked, this, &ToolbarPanelWidget::rectangleToolActivated);
    connect(m_createArcBtn, &QToolButton::clicked, this, &ToolbarPanelWidget::arcToolActivated);

    // Логика кнопки окружности:
    // По умолчанию (клик по иконке) - Центр, Радиус
    connect(m_createCircleBtn, &QToolButton::clicked, this, [this]() {
        emit circleToolActivated(CircleCreationMode::CenterRadius);
    });

    // Обработка пунктов меню
    // При выборе пункта меню мы также должны "нажать" саму кнопку визуально
    connect(actCenterRadius, &QAction::triggered, this, [this]() {
        m_createCircleBtn->click();
        emit circleToolActivated(CircleCreationMode::CenterRadius);
    });
    connect(actCenterDiameter, &QAction::triggered, this, [this]() {
        m_createCircleBtn->click();
        emit circleToolActivated(CircleCreationMode::CenterDiameter);
    });
    connect(act2Points, &QAction::triggered, this, [this]() {
        m_createCircleBtn->click();
        emit circleToolActivated(CircleCreationMode::TwoPoints);
    });
    connect(act3Points, &QAction::triggered, this, [this]() {
        m_createCircleBtn->click();
        emit circleToolActivated(CircleCreationMode::ThreePoints);
    });

    //минимальная ширина окна
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
