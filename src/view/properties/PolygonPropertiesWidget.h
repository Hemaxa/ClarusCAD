//PolygonPropertiesWidget - виджет свойств примитива "Многоугольник"

#pragma once

#include "BasePropertiesWidget.h"
#include "PointPrimitive.h"
#include "EnumManager.h"

#include <QLabel>

class QSpinBox;
class QComboBox;
class PolygonPrimitive;

//наследуется от базового класса BasePropertiesWidget
class PolygonPropertiesWidget : public BasePropertiesWidget
{
    Q_OBJECT

public:
    //конструктор
    explicit PolygonPropertiesWidget(QWidget* parent = nullptr);

    //переопределение метода установки объектов
    void setPrimitives(const QList<BasePrimitive*>& primitives) override;

signals:
    //сигнал для создания или обновления примитива "Многоугольник"
    void propertiesApplied(PolygonPrimitive* polygon, int sides, PolygonCreationMode type, 
                           const QColor& color, LineType lineType);
    
    //сигналы для обновления параметров инструмента в реальном времени
    void sidesChanged(int sides);
    void polygonTypeChanged(PolygonCreationMode type);

private slots:
    //слот для обработки нажатия кнопки "Создать"
    void onApplyButtonClicked();
    
    //слоты для обновления инструмента при изменении параметров
    void onSidesChanged(int value);
    void onTypeChanged(int index);

private:
    //реализация виртуальных методов из BasePropertiesWidget
    void updateFieldValues() override;
    void updatePrompt() override;

    //указатель на текущий редактируемый объект
    PolygonPrimitive* m_currentPolygon = nullptr;

    //поля объекта "Многоугольник"
    QSpinBox* m_sidesSpinBox;      //количество углов
    QComboBox* m_typeComboBox;     //тип (вписанный/описанный)
};
