//BasePropertiesWidget — базовый класс для всех виджетов свойств в приложении

#pragma once

#include <QWidget>
#include <QColor>

class BasePrimitive;
class QPushButton;
class QFormLayout;

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

protected slots:
    //слот нажатия на кнопку изменения цвета
    void onColorButtonClicked();

protected:
    //общие поля для всех примитивов
    BasePrimitive* m_currentPrimitive = nullptr;
    QColor m_selectedColor;
    QPushButton* m_applyButton;
    QPushButton* m_colorButton;
    QFormLayout* m_layout;
};
