#include "SceneSettingsPanelWidget.h"
#include "ThemeManager.h"
#include "AnimationManager.h"

#include <QToolButton>
#include <QGridLayout>
#include <QSpacerItem>

SceneSettingsPanelWidget::SceneSettingsPanelWidget(const QString& title, QWidget* parent) : BasePanelWidget(title, parent)
{
    //вертикальный шаблон компоновки, объекты прижимаются к верху
    auto* layout = new QGridLayout(canvas());

    layout->setContentsMargins(10, 10, 10, 10);

    //создание и добавление кнопок в группу
    //шаблон: текст описания, путь до иконки, горячая клавиша
    m_gridSnapBtn = new AnimationManager(":/icons/icons/grid-snap.svg", "Привязка к сетке [G]", Qt::Key_G);
    m_gridSnapBtn->setChecked(true);

    layout->addWidget(m_gridSnapBtn, 0, 0, Qt::AlignLeft);

    layout->setColumnStretch(1, 1);

    layout->setRowStretch(2, 1);

    //подключение сигналов от кнопок
    connect(m_gridSnapBtn, &QToolButton::toggled, this, &SceneSettingsPanelWidget::gridSnapToggled);

    //минимальная ширина окна
    setMinimumWidth(200);
}

void SceneSettingsPanelWidget::updateIcons()
{
    QColor iconColor = ThemeManager::instance().getIconColor();
    if (m_gridSnapBtn) {
        static_cast<AnimationManager*>(m_gridSnapBtn)->updateIconColor(iconColor);
    }
}
