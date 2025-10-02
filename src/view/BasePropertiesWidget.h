//BasePropertiesWidget — базовый класс для всех виджетов свойств в приложении

#pragma once

#include "EnumManager.h"

#include <QWidget>
#include <QColor>

class BasePrimitive;
class QPushButton;
class QFormLayout;
class QStackedWidget;
class QLabel;

class BasePropertiesWidget : public QWidget
{
    Q_OBJECT

public:
    //explicit запрещает неявное преобразование типов для конструктора
    explicit BasePropertiesWidget(QWidget* parent = nullptr);

    //virtual означает, что деструктор создается автоматически из унаследованного класса
    virtual ~BasePropertiesWidget() = default;

    //установка указателя на примитив, свойтва которого необходимо отображать
    virtual void setPrimitive(BasePrimitive* primitive);

    //установка типа системы координат
    virtual void setCoordinateSystem(CoordinateSystemType type);

    //метод обновления цвета
    void updateColor(const QColor& color);

signals:
    //сигнал, информирующий о выборе цвета для примитва
    void colorChanged(const QColor& color);

protected slots:
    //слот нажатия на кнопку изменения цвета
    void onColorButtonClicked();

protected:
    //метод обновления полей ввода
    virtual void updateFieldValues() = 0;

    //метод обнолвения подсказки
    virtual void updatePrompt();

    BasePrimitive* m_currentPrimitive = nullptr;
    CoordinateSystemType m_coordSystem;
    QColor m_selectedColor;

    //виджеты для колонок
    QLabel* m_leftColumn; //левая колонка
    QWidget* m_centralColumn; //центральная колонка
    QWidget* m_rightColumn; //правая колонка

    QStackedWidget* m_paramsStack; //общий виджет для сменяемых параметров центральной колонки
    QWidget* m_cartesianWidgets; //виджет декартовой системы координат
    QWidget* m_polarWidgets; //виджет полярной системы координат

    //кнопки
    QPushButton* m_colorButton;
    QPushButton* m_applyButton;
};
