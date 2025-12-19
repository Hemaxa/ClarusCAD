#include "SceneSettingsPanelWidget.h"
#include "AnimationManager.h"
#include "SnapManager.h"

#include <QButtonGroup>
#include <QToolButton>
#include <QGridLayout>
#include <QSpacerItem>
#include <QKeySequence>
#include <QLabel>

SceneSettingsPanelWidget::SceneSettingsPanelWidget(const QString& title, QWidget* parent) : BasePanelWidget(title, parent)
{
    //сетка для всех кнопок
    auto* layout = new QGridLayout(canvas());
    layout->setContentsMargins(10, 10, 10, 10);
    layout->setSpacing(5);

    //группа для кнопок систем координат
    auto* coordSystemGroup = new QButtonGroup(this);
    coordSystemGroup->setExclusive(true);

    //создание и добавление кнопок в группы
    //шаблон: путь до иконки, текст описания, горячая клавиша, залипание
    m_gridSnapBtn = new AnimationManager(":/icons/icons/scene/grid_snap.svg", "Привязка к сетке [G]", Qt::Key_G, true);
    m_gridSnapBtn->setChecked(true);

    m_primitiveSnapBtn = new AnimationManager(":/icons/icons/scene/primitive_snap.svg", "Привязка к объектам [O]", Qt::Key_O, true);
    m_primitiveSnapBtn->setChecked(true);
    
    // Расширенные привязки
    m_intersectionSnapBtn = new AnimationManager(":/icons/icons/scene/snap_intersection.svg", "Пересечение [I]", Qt::Key_I, true);
    m_intersectionSnapBtn->setChecked(true);
    
    m_perpendicularSnapBtn = new AnimationManager(":/icons/icons/scene/snap_perpendicular.svg", "Перпендикуляр [E]", Qt::Key_E, true);
    m_perpendicularSnapBtn->setChecked(true);
    
    m_tangentSnapBtn = new AnimationManager(":/icons/icons/scene/snap_tangent.svg", "Касательная [T]", Qt::Key_T, true);
    m_tangentSnapBtn->setChecked(true);

    m_cartesianBtn = new AnimationManager(":/icons/icons/scene/cartesian.svg", "Декартовы координаты [D]", Qt::Key_D, true);
    m_polarBtn = new AnimationManager(":/icons/icons/scene/polar.svg", "Полярные координаты [P]", Qt::Key_P, true);
    m_cartesianBtn->setChecked(true);

    //добавление кнопок систем координат в их группу
    coordSystemGroup->addButton(m_cartesianBtn);
    coordSystemGroup->addButton(m_polarBtn);

    //добавление кнопок в шаблон (без разделительных заголовков)
    layout->addWidget(m_gridSnapBtn, 0, 0, Qt::AlignLeft);
    layout->addWidget(m_primitiveSnapBtn, 1, 0, Qt::AlignLeft);
    layout->addWidget(m_intersectionSnapBtn, 2, 0, Qt::AlignLeft);
    layout->addWidget(m_perpendicularSnapBtn, 3, 0, Qt::AlignLeft);
    layout->addWidget(m_tangentSnapBtn, 4, 0, Qt::AlignLeft);
    layout->addWidget(m_cartesianBtn, 5, 0, Qt::AlignLeft);
    layout->addWidget(m_polarBtn, 6, 0, Qt::AlignLeft);

    //последняя пустая колонка должна растягиваться, прижимая кнопки влево
    layout->setColumnStretch(2, 1);

    //последняя путсая строка должна растягиваться, прижимая кнопки вверх
    layout->setRowStretch(7, 1);

    //подключение сигналов от кнопок
    connect(m_gridSnapBtn, &QToolButton::toggled, this, &SceneSettingsPanelWidget::gridSnapToggled);
    connect(m_primitiveSnapBtn, &QToolButton::toggled, this, &SceneSettingsPanelWidget::primitiveSnapToggled);
    connect(m_cartesianBtn, &QToolButton::clicked, this, [this]() { emit coordinateSystemChanged(CoordinateSystemType::Cartesian); });
    connect(m_polarBtn, &QToolButton::clicked, this, [this]() { emit coordinateSystemChanged(CoordinateSystemType::Polar); });
    
    // Сигналы расширенных привязок
    connect(m_intersectionSnapBtn, &QToolButton::toggled, this, &SceneSettingsPanelWidget::intersectionSnapToggled);
    connect(m_perpendicularSnapBtn, &QToolButton::toggled, this, &SceneSettingsPanelWidget::perpendicularSnapToggled);
    connect(m_tangentSnapBtn, &QToolButton::toggled, this, &SceneSettingsPanelWidget::tangentSnapToggled);

    //минимальная ширина окна
    setMinimumWidth(160);
}
