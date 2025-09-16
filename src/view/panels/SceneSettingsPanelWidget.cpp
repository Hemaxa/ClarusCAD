#include "SceneSettingsPanelWidget.h"
#include "ThemeManager.h"

#include <QToolButton>
#include <QVBoxLayout>
#include <QIcon>

SceneSettingsPanelWidget::SceneSettingsPanelWidget(const QString& title, QWidget* parent)
    : BasePanelWidget(title, parent)
{
    auto* layout = new QVBoxLayout(canvas());
    layout->setAlignment(Qt::AlignTop);

    m_gridSnapBtn = createToolButton("Привязка к сетке [G]", ":/icons/icons/grid_snap.svg", Qt::Key_G);
    connect(m_gridSnapBtn, &QToolButton::toggled, this, &SceneSettingsPanelWidget::gridSnapToggled);

    setMinimumWidth(100);
}

QToolButton* SceneSettingsPanelWidget::createToolButton(const QString& text, const QString& iconPath, const QKeySequence& shortcut)
{
    auto* button = new QToolButton();
    button->setToolTip(text);
    button->setCheckable(true);
    button->setChecked(true); // Привязка включена по умолчанию
    button->setIconSize(QSize(30, 30));
    button->setToolButtonStyle(Qt::ToolButtonIconOnly);
    button->setAutoRaise(true);
    button->setShortcut(shortcut);

    qobject_cast<QVBoxLayout*>(canvas()->layout())->addWidget(button);
    return button;
}

void SceneSettingsPanelWidget::updateIcons()
{
    QColor iconColor = ThemeManager::instance().getIconColor();
    if (m_gridSnapBtn) {
        m_gridSnapBtn->setIcon(ThemeManager::colorizeSvgIcon(":/icons/icons/grid_snap.svg", iconColor));
    }
}
