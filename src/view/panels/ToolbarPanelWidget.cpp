#include "ToolbarPanelWidget.h"
#include "ThemeManager.h"
#include "AnimationManager.h"

#include <QButtonGroup>
#include <QToolButton>
#include <QGridLayout>
#include <QSpacerItem>
#include <QKeySequence> //класс горячих клавиш Qt

ToolbarPanelWidget::ToolbarPanelWidget(const QString& title, QWidget* parent) : BasePanelWidget(title, parent)
{
    //сетка для всех кнопок
    auto* layout = new QGridLayout(canvas());
    layout->setContentsMargins(10, 10, 10, 10);

    //общая группа для кнопок на панели инструментов (для возможности выбора только одной)
    m_buttonGroup = new QButtonGroup(this); //создание группы кнопок (this - указатель на родителя для автоматического контроля памяти)
    m_buttonGroup->setExclusive(true); //возможность выбора только одной кнопки

    //создание и добавление кнопок в группу
    //шаблон: текст описания, путь до иконки, горячая клавиша
    m_deleteBtn = new AnimationManager(":/icons/icons/delete.svg", "Удаление [X]", Qt::Key_X);
    m_createSegmentBtn = new AnimationManager(":/icons/icons/segment.svg", "Отрезок [S]", Qt::Key_S);

    //добавление кнопок в общую группу
    m_buttonGroup->addButton(m_deleteBtn);
    m_buttonGroup->addButton(m_createSegmentBtn);

    //добавление кнопок в шаблон
    layout->addWidget(m_deleteBtn, 0, 0, Qt::AlignLeft);
    layout->addWidget(m_createSegmentBtn, 1, 0, Qt::AlignLeft);

    //последняя пустая колонка должна растягиваться, прижимая кнопки влево
    layout->setColumnStretch(1, 1);

    //последняя путсая строка должна растягиваться, прижимая кнопки вверх
    layout->setRowStretch(2, 1);

    //подключение сигналов от кнопок
    connect(m_deleteBtn, &QToolButton::clicked, this, &ToolbarPanelWidget::deleteToolActivated);
    connect(m_createSegmentBtn, &QToolButton::clicked, this, &ToolbarPanelWidget::segmentToolActivated);

    //минимальная ширина окна
    setMinimumWidth(200);
}

void ToolbarPanelWidget::updateIcons()
{
    //получение цвета иконок из менеджера тем
    QColor iconColor = ThemeManager::instance().getIconColor();

    //вызов метода перекрашивания иконки из AnimationManager
    if (m_deleteBtn) {
        static_cast<AnimationManager*>(m_deleteBtn)->updateIconColor(iconColor);
    }
    if (m_createSegmentBtn) {
        static_cast<AnimationManager*>(m_createSegmentBtn)->updateIconColor(iconColor);
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

QToolButton* ToolbarPanelWidget::getDeleteButton() const { return m_deleteBtn; }
QToolButton* ToolbarPanelWidget::getCreateSegmentButton() const { return m_createSegmentBtn; }
