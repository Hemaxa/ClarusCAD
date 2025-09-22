//SegmentPropertiesWidget - виджет свойств примитива "Отрезок"

#pragma once

#include "BasePropertiesWidget.h"
#include "PointPrimitive.h"

class QLineEdit;
class SegmentPrimitive;

//наслдедуется от базового класса BasePropertiesWidget
class SegmentPropertiesWidget : public BasePropertiesWidget
{
    Q_OBJECT

public:
    //конструктор
    explicit SegmentPropertiesWidget(QWidget* parent = nullptr);

    //реализация виртуального метода из BasePropertiesWidget
    void setPrimitive(BasePrimitive* primitive) override;

signals:
    //сигнал, для создания или обновления примитива "Отрезок"
    void propertiesApplied(SegmentPrimitive* segment, const PointPrimitive& start, const PointPrimitive& end, const QColor& color);

private slots:
    //слот для обработки нажатия кнопки "Создать"
    void onApplyButtonClicked();

private:
    //указатель на текущий редактируемый объект "Отрезок"
    SegmentPrimitive* m_currentSegment = nullptr;

    //параметры объекта "Отрезок"
    QLineEdit* m_startXEdit;
    QLineEdit* m_startYEdit;
    QLineEdit* m_endXEdit;
    QLineEdit* m_endYEdit;
};
