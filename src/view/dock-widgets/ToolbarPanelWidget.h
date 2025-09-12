//ToolbarPanelWidget - панель инструментов

#pragma once

#include "BaseDockWidget.h"

#include <QKeySequence>

class QToolButton;
class QButtonGroup;

//наслдедуется от базового класса BaseDockWidget
class ToolbarPanelWidget : public BaseDockWidget
{
    Q_OBJECT

public:
    //конструктор
    explicit ToolbarPanelWidget(const QString& title, QWidget* parent = nullptr);

signals:
    //сигналы нажатия кнопок
    void segmentToolActivated();

private:
    //метод создания и настройки кнопоки
    QToolButton* createToolButton(const QString& text, const QString& iconPath, const QKeySequence& shortcut);

    //группа для кнопок на панели инструментов
    QButtonGroup* m_buttonGroup;
};
