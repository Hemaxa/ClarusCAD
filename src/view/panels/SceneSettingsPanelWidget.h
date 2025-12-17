//SceneSettingsPanelWidget - панель настроек сцены

#pragma once

#include "BasePanelWidget.h"
#include "EnumManager.h"

class QToolButton;
class QButtonGroup;

//наследуется от базового класса BasePanelWidget
class SceneSettingsPanelWidget : public BasePanelWidget
{
    Q_OBJECT

public:
    //конструктор
    explicit SceneSettingsPanelWidget(const QString& title, QWidget* parent = nullptr);

signals:
    //сигналы нажатия соответствующих кнопок
    void gridSnapToggled(bool enabled);
    void primitiveSnapToggled(bool enabled);
    void coordinateSystemChanged(CoordinateSystemType type);
    
    //сигналы для расширенных типов привязок
    void intersectionSnapToggled(bool enabled);
    void perpendicularSnapToggled(bool enabled);
    void tangentSnapToggled(bool enabled);

private:
    //указатели на соответствующие кнопки
    QToolButton* m_gridSnapBtn;
    QToolButton* m_primitiveSnapBtn;
    QToolButton* m_cartesianBtn;
    QToolButton* m_polarBtn;
    
    //кнопки расширенных привязок
    QToolButton* m_intersectionSnapBtn;
    QToolButton* m_perpendicularSnapBtn;
    QToolButton* m_tangentSnapBtn;
};
