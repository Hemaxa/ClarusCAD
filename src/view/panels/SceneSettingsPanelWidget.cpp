#include "SceneSettingsPanelWidget.h"
#include "AnimationManager.h"

#include <QButtonGroup>
#include <QToolButton>
#include <QGridLayout>
#include <QSpacerItem>
#include <QKeySequence>

SceneSettingsPanelWidget::SceneSettingsPanelWidget(const QString& title, QWidget* parent) : BasePanelWidget(title, parent)
{
    //сетка для всех кнопок
    auto* layout = new QGridLayout(canvas());
    layout->setContentsMargins(10, 10, 10, 10);

    //группа для кнопок систем координат
    auto* coordSystemGroup = new QButtonGroup(this);
    coordSystemGroup->setExclusive(true);

    //создание и добавление кнопок в группы
    //шаблон: путь до иконки, текст описания, горячая клавиша, залипание
    m_gridSnapBtn = new AnimationManager(":/icons/icons/scene/grid-snap.svg", "Привязка к сетке [G]", Qt::Key_G, true);
    m_gridSnapBtn->setChecked(true);

    m_primitiveSnapBtn = new AnimationManager(":/icons/icons/scene/primitive-snap.svg", "Привязка к объектам [O]", Qt::Key_O, true);
    m_primitiveSnapBtn->setChecked(true);

    m_cartesianBtn = new AnimationManager(":/icons/icons/scene/cartesian.svg", "Декартовы координаты [D]", Qt::Key_D, true);
    m_polarBtn = new AnimationManager(":/icons/icons/scene/polar.svg", "Полярные координаты [P]", Qt::Key_P, true);
    m_cartesianBtn->setChecked(true);

    //добавление кнопок систем координат в их группу
    coordSystemGroup->addButton(m_cartesianBtn);
    coordSystemGroup->addButton(m_polarBtn);

    //добавление кнопок в шаблон
    layout->addWidget(m_gridSnapBtn, 0, 0, Qt::AlignLeft);
    layout->addWidget(m_primitiveSnapBtn, 1, 0, Qt::AlignLeft);
    layout->addWidget(m_cartesianBtn, 2, 0, Qt::AlignLeft);
    layout->addWidget(m_polarBtn, 3, 0, Qt::AlignLeft);

    //последняя пустая колонка должна растягиваться, прижимая кнопки влево
    layout->setColumnStretch(2, 1);

    //последняя путсая строка должна растягиваться, прижимая кнопки вверх
    layout->setRowStretch(4, 1);

    //подключение сигналов от кнопок
    connect(m_gridSnapBtn, &QToolButton::toggled, this, &SceneSettingsPanelWidget::gridSnapToggled);
    connect(m_primitiveSnapBtn, &QToolButton::toggled, this, &SceneSettingsPanelWidget::primitiveSnapToggled);
    connect(m_cartesianBtn, &QToolButton::clicked, this, [this]() { emit coordinateSystemChanged(CoordinateSystemType::Cartesian); });
    connect(m_polarBtn, &QToolButton::clicked, this, [this]() { emit coordinateSystemChanged(CoordinateSystemType::Polar); });

    //минимальная ширина окна
    setMinimumWidth(200);
}
