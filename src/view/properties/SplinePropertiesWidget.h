//SplinePropertiesWidget - виджет свойств примитива "Сплайн"

#pragma once

#include "BasePropertiesWidget.h"
#include "PointPrimitive.h"

#include <QLabel>

class QCheckBox;
class SplinePrimitive;

//наследуется от базового класса BasePropertiesWidget
class SplinePropertiesWidget : public BasePropertiesWidget
{
    Q_OBJECT

public:
    //конструктор
    explicit SplinePropertiesWidget(QWidget* parent = nullptr);

    //переопределение метода установки объектов
    void setPrimitives(const QList<BasePrimitive*>& primitives) override;

signals:
    //сигнал для создания или обновления примитива "Сплайн"
    void propertiesApplied(SplinePrimitive* spline, bool closed, 
                           const QColor& color, LineType lineType);
    
    //сигнал для обновления параметра инструмента
    void closedChanged(bool closed);

private slots:
    //слот для обработки нажатия кнопки "Создать"
    void onApplyButtonClicked();
    
    //слот для обновления инструмента при изменении параметра
    void onClosedChanged(bool checked);

private:
    //реализация виртуальных методов из BasePropertiesWidget
    void updateFieldValues() override;
    void updatePrompt() override;

    //указатель на текущий редактируемый объект
    SplinePrimitive* m_currentSpline = nullptr;

    //поля объекта "Сплайн"
    QCheckBox* m_closedCheckBox;   //замкнутый/разомкнутый
};
