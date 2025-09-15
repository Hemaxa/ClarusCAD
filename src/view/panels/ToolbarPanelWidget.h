//ToolbarPanelWidget - панель инструментов

#pragma once

#include "BasePanelWidget.h"

#include <QKeySequence> //класс горячих клавиш Qt

class QToolButton;
class QButtonGroup;

//наслдедуется от базового класса BasePanelWidget
class ToolbarPanelWidget : public BasePanelWidget
{
    Q_OBJECT

public:
    //конструктор
    explicit ToolbarPanelWidget(const QString& title, QWidget* parent = nullptr);

    //метод обновления иконок (перекрашивание)
    void updateIcons();

    //метод снятия выделения с инструментов
    void clearSelection();

signals:
    //сигналы нажатия соответствующих кнопок
    void deleteToolActivated();
    void segmentToolActivated();

private:
    //метод создания и настройки кнопок
    QToolButton* createToolButton(const QString& text, const QString& iconPath, const QKeySequence& shortcut);

    //группа для кнопок на панели инструментов
    QButtonGroup* m_buttonGroup;

    QToolButton* m_createSegmentBtn;
};
