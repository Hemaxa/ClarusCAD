#include "ToolbarPanelWidget.h"
#include "ThemeManager.h"

#include <QToolButton>
#include <QButtonGroup>
#include <QVBoxLayout>
#include <QIcon>

ToolbarPanelWidget::ToolbarPanelWidget(const QString& title, QWidget* parent) : BasePanelWidget(title, parent)
{
    //вертикальный шаблон компоновки, объекты прижимаются к верху
    auto* layout = new QVBoxLayout(canvas());
    layout->setAlignment(Qt::AlignTop);

    m_buttonGroup = new QButtonGroup(this); //создание группы кнопок (this - указатель на родителя для автоматического контроля памяти)
    m_buttonGroup->setExclusive(true); //возможность выбора только одной кнопки

    //создание и добавление кнопок в группу
    //шаблон: текст описания, путь до иконки, горячая клавиша
    auto* deleteBtn = createToolButton("Удалить [Del]", ":/icons/icons/delete.svg", Qt::Key_Delete);
    auto* createSegmentBtn = createToolButton("Отрезок [S]", ":/icons/icons/segment.svg", Qt::Key_S);

    //подключение сигналов от кнопок
    connect(deleteBtn, &QToolButton::clicked, this, &ToolbarPanelWidget::deleteToolActivated);
    connect(createSegmentBtn, &QToolButton::clicked, this, &ToolbarPanelWidget::segmentToolActivated);

    //минимальная ширина окна
    setMinimumWidth(200);
}

QToolButton* ToolbarPanelWidget::createToolButton(const QString& text, const QString& iconPath, const QKeySequence& shortcut)
{
    //создание кнопки
    auto* button = new QToolButton();
    button->setToolTip(text); //текст для описания кнопки
    button->setCheckable(true); //"залипающее" поведение кнопки

    // Получаем цвет из менеджера и применяем его к иконке
    QColor iconColor = ThemeManager::instance().getIconColor();
    button->setIcon(ThemeManager::colorizeSvgIcon(iconPath, iconColor));

    //установка иконки
    button->setIconSize(QSize(30, 30)); //размер иконки

    button->setToolButtonStyle(Qt::ToolButtonIconOnly); //показывается только иконка, без текста
    button->setAutoRaise(true);

    //установка горячей клавиши
    button->setShortcut(shortcut);

    //добавление кнопки на панель и в группу
    qobject_cast<QVBoxLayout*>(canvas()->layout())->addWidget(button);
    m_buttonGroup->addButton(button);

    return button;
}

void ToolbarPanelWidget::updateIcons()
{
    QColor iconColor = ThemeManager::instance().getIconColor();
    if (m_createSegmentBtn) {
        m_createSegmentBtn->setIcon(ThemeManager::colorizeSvgIcon(":/icons/icons/segment.svg", iconColor));
    }
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
