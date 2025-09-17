#include "SceneSettingsPanelWidget.h"
#include "ThemeManager.h"

#include <QToolButton>
#include <QVBoxLayout>
#include <QIcon>

SceneSettingsPanelWidget::SceneSettingsPanelWidget(const QString& title, QWidget* parent) : BasePanelWidget(title, parent)
{
    //вертикальный шаблон компоновки, объекты прижимаются к верху
    auto* layout = new QVBoxLayout(canvas());
    layout->setAlignment(Qt::AlignTop);

    //создание и добавление кнопок в группу
    //шаблон: текст описания, путь до иконки, горячая клавиша
    m_gridSnapBtn = createToolButton("Привязка к сетке [G]", ":/icons/icons/grid_snap.svg", Qt::Key_G);

    //подключение сигналов от кнопок
    connect(m_gridSnapBtn, &QToolButton::toggled, this, &SceneSettingsPanelWidget::gridSnapToggled);

    //минимальная ширина окна
    setMinimumWidth(200);
}

QToolButton* SceneSettingsPanelWidget::createToolButton(const QString& text, const QString& iconPath, const QKeySequence& shortcut)
{
    //создание кнопки
    auto* button = new QToolButton();
    button->setToolTip(text); //текст для описания кнопки
    button->setCheckable(true); //"залипающее" поведение кнопки

    //получение цвета из менеджера и применение его к иконке
    QColor iconColor = ThemeManager::instance().getIconColor();
    button->setIcon(ThemeManager::colorizeSvgIcon(iconPath, iconColor));

    //активация по умолчанию
    button->setChecked(true);

    //установка иконки
    button->setIconSize(QSize(30, 30)); //размер иконки

    button->setToolButtonStyle(Qt::ToolButtonIconOnly); //показывается только иконка, без текста
    button->setAutoRaise(true);

    //установка горячей клавиши
    button->setShortcut(shortcut);

    //добавление кнопки на панель
    qobject_cast<QVBoxLayout*>(canvas()->layout())->addWidget(button);

    //возвращает указатель на кнопку
    return button;
}

void SceneSettingsPanelWidget::updateIcons()
{
    //получение цвета иконок из файла темы
    QColor iconColor = ThemeManager::instance().getIconColor();
    //перекрашивание существующих кнопок
    if (m_gridSnapBtn) {
        m_gridSnapBtn->setIcon(ThemeManager::colorizeSvgIcon(":/icons/icons/grid_snap.svg", iconColor));
    }
}
