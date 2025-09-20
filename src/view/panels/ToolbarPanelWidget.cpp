#include "ToolbarPanelWidget.h"
#include "ThemeManager.h"
#include "AnimationManager.h"

#include <QToolButton>
#include <QButtonGroup>
#include <QGridLayout>
#include <QSpacerItem>

ToolbarPanelWidget::ToolbarPanelWidget(const QString& title, QWidget* parent) : BasePanelWidget(title, parent)
{
    //вертикальный шаблон компоновки, объекты прижимаются к верху
    auto* layout = new QGridLayout(canvas());

    layout->setContentsMargins(10, 10, 10, 10);

    m_buttonGroup = new QButtonGroup(this); //создание группы кнопок (this - указатель на родителя для автоматического контроля памяти)
    m_buttonGroup->setExclusive(true); //возможность выбора только одной кнопки

    //создание и добавление кнопок в группу
    //шаблон: текст описания, путь до иконки, горячая клавиша
    m_deleteBtn = new AnimationManager(":/icons/icons/delete.svg", "Удалить [X]", Qt::Key_X);
    m_createSegmentBtn = new AnimationManager(":/icons/icons/segment.svg", "Отрезок [S]", Qt::Key_S);

    // --- ИЗМЕНЕНИЯ ЗДЕСЬ ---
    // 1. Выравниваем кнопки по левому краю ячейки, а не по центру
    layout->addWidget(m_deleteBtn, 0, 0, Qt::AlignLeft);
    layout->addWidget(m_createSegmentBtn, 1, 0, Qt::AlignLeft);

    // 2. Указываем, что вторая колонка (с индексом 1) должна растягиваться,
    // тем самым прижимая первую колонку (с кнопками) влево.
    layout->setColumnStretch(1, 1);

    // 3. Указываем, что третья строка (с индексом 2) должна растягиваться,
    // прижимая первые две строки (с кнопками) вверх.
    layout->setRowStretch(2, 1);

    // 2. Добавляем кнопки в группу, чтобы работало эксклюзивное выделение
    m_buttonGroup->addButton(m_deleteBtn);
    m_buttonGroup->addButton(m_createSegmentBtn);

    //подключение сигналов от кнопок
    connect(m_deleteBtn, &QToolButton::clicked, this, &ToolbarPanelWidget::deleteToolActivated);
    connect(m_createSegmentBtn, &QToolButton::clicked, this, &ToolbarPanelWidget::segmentToolActivated);

    //минимальная ширина окна
    setMinimumWidth(200);
}

void ToolbarPanelWidget::updateIcons()
{
    QColor iconColor = ThemeManager::instance().getIconColor();

    if (m_deleteBtn) {
        // Безопасно обновляем цвет через новый метод
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
