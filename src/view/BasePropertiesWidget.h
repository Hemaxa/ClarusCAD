//BasePropertiesWidget — базовый класс для всех остальных виджетов свойств в приложении

#pragma once

#include <QWidget>

class BasePrimitive;

class BasePropertiesWidget : public QWidget
{
    Q_OBJECT

public:
    //explicit запрещает неявное преобразование типов для конструктора
    explicit BasePropertiesWidget(QWidget* parent = nullptr) : QWidget(parent) {}

    //virtual означает, что деструктор создается автоматически из унаследованного класса
    virtual ~BasePropertiesWidget() = default;

    //установка указателя на примитив, свойтва которого необходимо отображать
    virtual void setPrimitive(BasePrimitive* primitive) = 0;
};
