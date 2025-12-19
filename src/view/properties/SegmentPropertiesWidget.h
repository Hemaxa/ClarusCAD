//SegmentPropertiesWidget - виджет свойств примитива "Отрезок"

#pragma once

#include "BasePropertiesWidget.h"
#include "PointPrimitive.h"

#include <QLabel>

class QLineEdit;
class SegmentPrimitive;

//наслдедуется от базового класса BasePropertiesWidget
class SegmentPropertiesWidget : public BasePropertiesWidget
{
    Q_OBJECT

public:
    //конструктор
    explicit SegmentPropertiesWidget(QWidget* parent = nullptr);

    //переопределение метода установки объектов
    void setPrimitives(const QList<BasePrimitive*>& primitives) override;

signals:
    //сигнал, для создания или обновления примитива "Отрезок"
    void propertiesApplied(SegmentPrimitive* segment, const PointPrimitive& start, const PointPrimitive& end, const QColor& color, LineType lineType);

private slots:
    //слот для обработки нажатия кнопки "Создать"
    void onApplyButtonClicked();

private:
    //реализация виртуального метода заполнения значениями полей ввода из BasePropertiesWidget
    void updateFieldValues() override;

    //указатель на текущий редактируемый объект "Отрезок" (главный из выделенных)
    SegmentPrimitive* m_currentSegment = nullptr;

    //поля объекта "Отрезок"
    //декартова система координат
    QLineEdit* m_startXEdit;
    QLineEdit* m_startYEdit;
    QLineEdit* m_endXEdit;
    QLineEdit* m_endYEdit;

    //полярная система координат
    QLineEdit* m_startRadiusEdit;
    QLineEdit* m_startAngleEdit;
    QLineEdit* m_endRadiusEdit;
    QLineEdit* m_endAngleEdit;
};
