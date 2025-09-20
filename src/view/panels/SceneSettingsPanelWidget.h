//SceneSettingsPanelWidget - панель настроек сцены

#pragma once

#include "BasePanelWidget.h"

class QToolButton;

//наслдедуется от базового класса BasePanelWidget
class SceneSettingsPanelWidget : public BasePanelWidget
{
    Q_OBJECT

public:
    //конструктор
    explicit SceneSettingsPanelWidget(const QString& title, QWidget* parent = nullptr);

    //метод обновления иконок (перекрашивание)
    void updateIcons();

signals:
    //сигналы нажатия соответствующих кнопок
    void gridSnapToggled(bool enabled);

private:
    //указатели на соответствующие кнопки
    QToolButton* m_gridSnapBtn;
};
