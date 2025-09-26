//BasePropertiesWidget — базовый класс для всех виджетов свойств в приложении

#pragma once

#include "EnumManager.h"

#include <QWidget>
#include <QColor>

class BasePrimitive;
class QPushButton;
class QFormLayout;
class QStackedWidget;

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
    virtual void updateFieldsVisibility();
    virtual void updateFieldValues() = 0;

    //общие поля для всех примитивов
    BasePrimitive* m_currentPrimitive = nullptr;
    CoordinateSystemType m_coordSystem;
    QColor m_selectedColor;
    QPushButton* m_applyButton;
    QPushButton* m_colorButton;
    QStackedWidget* m_stack;
    QWidget* m_cartesianWidgets;
    QWidget* m_polarWidgets;
    QWidget* m_basePref;
};
