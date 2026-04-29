#include "BasePanelWidget.h"
#include "ThemeManager.h"
#include "AnimationManager.h"
#include "FlyoutToolButton.h"

#include <QColor>

BasePanelWidget::BasePanelWidget(const QString& title, QWidget* parent = nullptr) : QDockWidget(title, parent)
{
    //создание пустого универсального холста для окна
    m_canvas = new QWidget();

    //установка холста в качестве основного виджета QDockWidget
    setWidget(m_canvas);
}

void BasePanelWidget::updateColors()
{
    //получение цвета из менеджера тем
    QColor iconColor = ThemeManager::instance().getIconColor();

    //получение всех дочерних элементов виджета AnimationManager
    auto buttons = this->findChildren<AnimationManager*>();

    //перекрашивание элементов в цикле
    for (AnimationManager* button : buttons) {
        if (button) {
            button->updateIconColor(iconColor);
        }
    }

    const auto flyoutButtons = this->findChildren<FlyoutToolButton*>();
    for (FlyoutToolButton* button : flyoutButtons) {
        if (button) {
            button->updateColors();
        }
    }
}

QWidget* BasePanelWidget::canvas() const
{
    return m_canvas;
}
