#include "BasePanelWidget.h"
#include "ThemeManager.h"
#include "AnimationManager.h"

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
    // 1. Получаем цвет
    QColor iconColor = ThemeManager::instance().getIconColor();

    // 2. Находим ВСЕ дочерние виджеты типа AnimationManager
    auto buttons = this->findChildren<AnimationManager*>();

    // 3. Перекрашиваем их в цикле
    for (AnimationManager* button : buttons) {
        if (button) {
            button->updateIconColor(iconColor);
        }
    }
}

QWidget* BasePanelWidget::canvas() const
{
    return m_canvas;
}
