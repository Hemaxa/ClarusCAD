#include "SceneSettingsPanelWidget.h"
#include "ThemeManager.h"
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
    //шаблон: текст описания, путь до иконки, горячая клавиша
    m_gridSnapBtn = new AnimationManager(":/icons/icons/grid-snap.svg", "Привязка к сетке [G]", Qt::Key_G);
    m_gridSnapBtn->setChecked(true);

    m_primitiveSnapBtn = new AnimationManager(":/icons/icons/primitive-snap.svg", "Привязка к объектам [O]", Qt::Key_O);
    m_primitiveSnapBtn->setChecked(true);

    m_cartesianBtn = new AnimationManager(":/icons/icons/cartesian.svg", "Декартовы координаты [D]", Qt::Key_D);
    m_polarBtn = new AnimationManager(":/icons/icons/polar.svg", "Полярные координаты [P]", Qt::Key_P);
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
    layout->setColumnStretch(1, 1);

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

void SceneSettingsPanelWidget::updateIcons()
{
    //получение цвета иконок из менеджера тем
    QColor iconColor = ThemeManager::instance().getIconColor();

    //вызов метода перекрашивания иконки из AnimationManager
    if (m_gridSnapBtn) {
        static_cast<AnimationManager*>(m_gridSnapBtn)->updateIconColor(iconColor);
    }
    if (m_primitiveSnapBtn) {
        static_cast<AnimationManager*>(m_primitiveSnapBtn)->updateIconColor(iconColor);
    }
    if (m_cartesianBtn) {
        static_cast<AnimationManager*>(m_cartesianBtn)->updateIconColor(iconColor);
    }
    if (m_polarBtn) {
        static_cast<AnimationManager*>(m_polarBtn)->updateIconColor(iconColor);
    }
}
