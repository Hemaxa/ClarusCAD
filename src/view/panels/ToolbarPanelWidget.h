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

signals:
    //сигналы нажатия соответствующих кнопок
    void segmentToolActivated();

private:
    //метод создания и настройки кнопок
    QToolButton* createToolButton(const QString& text, const QString& iconPath, const QKeySequence& shortcut);

    //группа для кнопок на панели инструментов
    QButtonGroup* m_buttonGroup;
};
