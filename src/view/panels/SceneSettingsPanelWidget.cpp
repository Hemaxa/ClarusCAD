#include "SceneSettingsPanelWidget.h"
#include "ThemeManager.h"
#include "AnimationManager.h"

#include <QToolButton>
#include <QButtonGroup>
#include <QGridLayout>
#include <QSpacerItem>

SceneSettingsPanelWidget::SceneSettingsPanelWidget(const QString& title, QWidget* parent) : BasePanelWidget(title, parent)
{
    //сетка для всех кнопок
    auto* layout = new QGridLayout(canvas());
    layout->setContentsMargins(10, 10, 10, 10);

    //группа кнопок переключения систем координат
    auto* coordSystemGroup = new QButtonGroup(this);
    coordSystemGroup->setExclusive(true);

    //создание и добавление кнопок в группы
    //шаблон: текст описания, путь до иконки, горячая клавиша
    m_gridSnapBtn = new AnimationManager(":/icons/icons/grid-snap.svg", "Привязка к сетке [G]", Qt::Key_G);
    m_gridSnapBtn->setChecked(true);

    m_cartesianBtn = new AnimationManager(":/icons/icons/cartesian.svg", "Декартовы координаты [D]", Qt::Key_D);
    m_polarBtn = new AnimationManager(":/icons/icons/polar.svg", "Полярные координаты [P]", Qt::Key_P);
    m_cartesianBtn->setChecked(true);

    coordSystemGroup->addButton(m_cartesianBtn);
    coordSystemGroup->addButton(m_polarBtn);

    layout->addWidget(m_gridSnapBtn, 0, 0, Qt::AlignLeft);
    layout->addWidget(m_cartesianBtn, 1, 0, Qt::AlignLeft);
    layout->addWidget(m_polarBtn, 2, 0, Qt::AlignLeft);

    layout->setColumnStretch(1, 1);
    layout->setRowStretch(3, 1);

    //подключение сигналов от кнопок
    connect(m_gridSnapBtn, &QToolButton::toggled, this, &SceneSettingsPanelWidget::gridSnapToggled);
    connect(m_cartesianBtn, &QToolButton::clicked, this, [this]() { emit coordinateSystemChanged(CoordinateSystemType::Cartesian); });
    connect(m_polarBtn, &QToolButton::clicked, this, [this]() { emit coordinateSystemChanged(CoordinateSystemType::Polar); });

    //минимальная ширина окна
    setMinimumWidth(200);
}

void SceneSettingsPanelWidget::updateIcons()
{
    QColor iconColor = ThemeManager::instance().getIconColor();
    if (m_gridSnapBtn) {
        static_cast<AnimationManager*>(m_gridSnapBtn)->updateIconColor(iconColor);
    }
    if (m_cartesianBtn) {
        static_cast<AnimationManager*>(m_cartesianBtn)->updateIconColor(iconColor);
    }
    if (m_polarBtn) {
        static_cast<AnimationManager*>(m_polarBtn)->updateIconColor(iconColor);
    }
}
