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
    /**
     * @brief Конструктор панели настроек сцены.
     */
    explicit SceneSettingsPanelWidget(const QString& title, QWidget* parent = nullptr);

signals:
    // Основные настройки
    
    /**
     * @brief Сигнал переключения привязки к сетке.
     */
    void gridSnapToggled(bool enabled);

    /**
     * @brief Сигнал переключения привязки к примитивам.
     */
    void primitiveSnapToggled(bool enabled);

    /**
     * @brief Сигнал смены системы координат.
     */
    void coordinateSystemChanged(CoordinateSystemType type);
    
    // Расширенные привязки
    
    /**
     * @brief Сигнал переключения привязки к пересечениям.
     */
    void intersectionSnapToggled(bool enabled);

    /**
     * @brief Сигнал переключения привязки к перпендикулярам.
     */
    void perpendicularSnapToggled(bool enabled);

    /**
     * @brief Сигнал переключения привязки к касательным.
     */
    void tangentSnapToggled(bool enabled);

private:
    // Кнопки основных настроек
    QToolButton* m_gridSnapBtn;
    QToolButton* m_primitiveSnapBtn;
    QToolButton* m_cartesianBtn;
    QToolButton* m_polarBtn;
    
    // Кнопки расширенных привязок
    QToolButton* m_intersectionSnapBtn;
    QToolButton* m_perpendicularSnapBtn;
    QToolButton* m_tangentSnapBtn;
};
