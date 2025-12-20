//BasePropertiesWidget — базовый класс для всех виджетов свойств в приложении

#pragma once

#include "EnumManager.h"

#include <QWidget>
#include <QColor>
#include <QList>

class BasePrimitive;
class QPushButton;
class QFormLayout;
class QStackedWidget;
class QLabel;
class QComboBox;

class BasePropertiesWidget : public QWidget
{
    Q_OBJECT

public:
    /**
     * @brief Конструктор виджета свойств.
     */
    explicit BasePropertiesWidget(QWidget* parent = nullptr);

    /**
     * @brief Виртуальный деструктор.
     */
    virtual ~BasePropertiesWidget() = default;

    /**
     * @brief Установить список примитивов для редактирования.
     * Поддерживает множественное выделение.
     * @param primitives Список указателей на примитивы.
     */
    virtual void setPrimitives(const QList<BasePrimitive*>& primitives);

    /**
     * @brief Переключить отображаемую систему координат.
     * @param type Тип системы координат (Декартова/Полярная).
     */
    virtual void setCoordinateSystem(CoordinateSystemType type);

    /**
     * @brief Обновить цвет в интерфейсе (вызывается извне).
     * @param color Новый цвет.
     * @param isMixed Если true, значит выбрано несколько объектов с разными цветами.
     */
    void updateColor(const QColor& color, bool isMixed = false);

    /**
     * @brief Обновить тип линии в интерфейсе (вызывается извне).
     * @param typeId ID типа линии.
     * @param isMixed Если true, значит выбрано несколько объектов с разными типами линий.
     */
    void updateLineType(int typeId, bool isMixed = false);

signals:
    /**
     * @brief Сигнал изменения цвета пользователем.
     * @param color Выбранный цвет.
     */
    void colorChanged(const QColor& color);

    /**
     * @brief Сигнал изменения типа линии пользователем.
     * @param type Выбранный тип линии.
     */
    void lineTypeChanged(LineType type); //Оставляем LineType для обратной совместимости сигналов

public slots:
    /**
     * @brief Слот, вызываемый при смене темы для перекраски иконок.
     */
    void updateColors();

protected slots:
    /**
     * @brief Слот обработки нажатия на кнопку выбора цвета.
     */
    void onColorButtonClicked();

    /**
     * @brief Слот обработки выбора типа линии из выпадающего списка.
     * @param index Индекс выбранного элемента.
     */
    void onLineTypeBoxClicked(int index);

protected:
    /**
     * @brief Чистый виртуальный метод обновления значений полей ввода.
     * Реализуется в конкретных виджетах (круг, отрезок и т.д.) для заполнения полей данными из m_currentPrimitive.
     */
    virtual void updateFieldValues() = 0;

    /**
     * @brief Заполнить выпадающий список типов линий доступными стилями.
     */
    void populateLineTypeComboBox();

    QList<BasePrimitive*> m_selectedPrimitives; ///< Список редактируемых объектов
    BasePrimitive* m_currentPrimitive = nullptr; ///< Указатель на "главный" объект (обычно последний) для чтения геометрии

    CoordinateSystemType m_selectedCoordSystem;
    QColor m_selectedColor;
    int m_selectedLineTypeId; //int для поддержки custom styles

    //виджеты для колонок
    QWidget* m_centralColumn; //центральная колонка (параметры)
    QWidget* m_rightColumn; //правая колонка (цвет/тип линии)

    QStackedWidget* m_paramsStack; //общий виджет для сменяемых параметров центральной колонки
    QWidget* m_cartesianWidgets; //виджет декартовой системы координат
    QWidget* m_polarWidgets; //виджет полярной системы координат

    //интерфейсы взаимодействия
    QPushButton* m_colorButton;
    QComboBox* m_lineTypeComboBox;
    QPushButton* m_applyButton;
};
