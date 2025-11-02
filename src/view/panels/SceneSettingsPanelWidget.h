//SceneSettingsPanelWidget - панель настроек сцены

#pragma once

#include "BasePanelWidget.h"
#include "EnumManager.h"

class QToolButton;
class QButtonGroup;

//наслдедуется от базового класса BasePanelWidget
class SceneSettingsPanelWidget : public BasePanelWidget
{
    Q_OBJECT

public:
    //конструктор
    explicit SceneSettingsPanelWidget(const QString& title, QWidget* parent = nullptr);

    //метод обновления иконок (перекрашивание)
    void updateColors();

signals:
    //сигналы нажатия соответствующих кнопок
    void gridSnapToggled(bool enabled);
    void primitiveSnapToggled(bool enabled);
    void coordinateSystemChanged(CoordinateSystemType type);
    void zoomInClicked();
    void zoomOutClicked();

private:
    //указатели на соответствующие кнопки
    QToolButton* m_gridSnapBtn;
    QToolButton* m_primitiveSnapBtn;
    QToolButton* m_cartesianBtn;
    QToolButton* m_polarBtn;
    QToolButton* m_zoomInBtn;
    QToolButton* m_zoomOutBtn;
};
