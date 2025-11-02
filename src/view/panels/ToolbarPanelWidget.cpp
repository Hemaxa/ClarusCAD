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
    //шаблон: путь до иконки, текст описания, горячая клавиша, залипание
    m_deleteBtn = new AnimationManager(":/icons/icons/tools/delete.svg", "Удаление [X]", Qt::Key_X, true);
    m_moveBtn = new AnimationManager(":/icons/icons/tools/move.svg", "Перемещение [M]", Qt::Key_M, true);
    m_createSegmentBtn = new AnimationManager(":/icons/icons/tools/segment.svg", "Отрезок [S]", Qt::Key_S, true);

    //добавление кнопок в общую группу
    m_buttonGroup->addButton(m_deleteBtn);
    m_buttonGroup->addButton(m_moveBtn);
    m_buttonGroup->addButton(m_createSegmentBtn);

    //добавление кнопок в шаблон
    layout->addWidget(m_deleteBtn, 0, 0, Qt::AlignLeft);
    layout->addWidget(m_moveBtn, 1, 0, Qt::AlignLeft);
    layout->addWidget(m_createSegmentBtn, 2, 0, Qt::AlignLeft);

    //последняя пустая колонка должна растягиваться, прижимая кнопки влево
    layout->setColumnStretch(1, 1);

    //последняя путсая строка должна растягиваться, прижимая кнопки вверх
    layout->setRowStretch(3, 1);

    //подключение сигналов от кнопок
    connect(m_deleteBtn, &QToolButton::clicked, this, &ToolbarPanelWidget::deleteToolActivated);
    connect(m_moveBtn, &QToolButton::clicked, this, &ToolbarPanelWidget::moveToolActivated);
    connect(m_createSegmentBtn, &QToolButton::clicked, this, &ToolbarPanelWidget::segmentToolActivated);

    //минимальная ширина окна
    setMinimumWidth(200);
}

// void ToolbarPanelWidget::updateColors()
// {
//     //получение цвета иконок из менеджера тем
//     QColor iconColor = ThemeManager::instance().getIconColor();

//     //вызов метода перекрашивания иконки из AnimationManager
//     if (m_deleteBtn) {
//         static_cast<AnimationManager*>(m_deleteBtn)->updateIconColor(iconColor);
//     }
//     if (m_moveBtn) {
//         static_cast<AnimationManager*>(m_moveBtn)->updateIconColor(iconColor);
//     }
//     if (m_createSegmentBtn) {
//         static_cast<AnimationManager*>(m_createSegmentBtn)->updateIconColor(iconColor);
//     }
// }

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
QToolButton* ToolbarPanelWidget::getMoveButton() const { return m_moveBtn; }
QToolButton* ToolbarPanelWidget::getCreateSegmentButton() const { return m_createSegmentBtn; }
