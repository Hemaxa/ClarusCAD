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
    //explicit запрещает неявное преобразование типов для конструктора
    explicit BasePropertiesWidget(QWidget* parent = nullptr);

    //virtual означает, что деструктор создается автоматически из унаследованного класса
    virtual ~BasePropertiesWidget() = default;

    //установка списка примитивов для редактирования (множественное выделение)
    virtual void setPrimitives(const QList<BasePrimitive*>& primitives);

    //установка типа системы координат
    virtual void setCoordinateSystem(CoordinateSystemType type);

    //метод обновления цвета
    void updateColor(const QColor& color, bool isMixed = false);

    //метод обновления типа линии
    void updateLineType(int typeId, bool isMixed = false);

signals:
    //сигнал, информирующий о выборе цвета для примитва
    void colorChanged(const QColor& color);

    //сигнал, информирующий о выборе типа линии для примитва
    void lineTypeChanged(LineType type); //Оставляем LineType для обратной совместимости сигналов

public slots:
    //слот, вызываемый при смене темы для перекраски всех иконок
    void updateColors();

protected slots:
    //слот нажатия на кнопку изменения цвета
    void onColorButtonClicked();

    //слот нажатия на панель выбора типа линии
    void onLineTypeBoxClicked(int index);

protected:
    //метод обновления полей ввода
    virtual void updateFieldValues() = 0;

    //метод обнолвения подсказки (тип примитива и применная тема)
    virtual void updatePrompt();

    //метод заполнения вкладки с типами линий
    void populateLineTypeComboBox();

    //список редактируемых объектов
    QList<BasePrimitive*> m_selectedPrimitives;
    BasePrimitive* m_currentPrimitive = nullptr; //Указатель на "главный" объект (обычно последний) для чтения геометрии

    CoordinateSystemType m_selectedCoordSystem;
    QColor m_selectedColor;
    int m_selectedLineTypeId; //int для поддержки custom styles

    //виджеты для колонок
    QLabel* m_leftColumn; //левая колонка
    QWidget* m_centralColumn; //центральная колонка
    QWidget* m_rightColumn; //правая колонка

    QStackedWidget* m_paramsStack; //общий виджет для сменяемых параметров центральной колонки
    QWidget* m_cartesianWidgets; //виджет декартовой системы координат
    QWidget* m_polarWidgets; //виджет полярной системы координат

    //интерфейсы взаимодействия
    QPushButton* m_colorButton;
    QComboBox* m_lineTypeComboBox;
    QPushButton* m_applyButton;
};
