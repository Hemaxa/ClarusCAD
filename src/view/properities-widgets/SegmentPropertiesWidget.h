//SegmentPropertiesWidget - виджет свойств примитива "Отрезок"

#pragma once

#include "BasePropertiesWidget.h"
#include "PointCreationPrimitive.h"

class QLineEdit;
class SegmentCreationPrimitive;

//наслдедуется от базового класса BasePropertiesWidget
class SegmentPropertiesWidget : public BasePropertiesWidget
{
    Q_OBJECT

signals:
    //сигнал, который отправляется при нажатии кнопки "Создать" в PropertiesPanelWidget
    void createSegmentRequested(const PointCreationPrimitive& start, const PointCreationPrimitive& end);

private slots:
    //слот для обработки нажатия кнопки "Создать"
    void onCreateButtonClicked();

public:
    //конструктор
    explicit SegmentPropertiesWidget(QWidget* parent = nullptr);

    //реализация виртуального метода из BasePropertiesWidget
    void setPrimitive(BasePrimitive* primitive) override;

private:
    //указатель на текущий редактируемый объект "Отрезок"
    SegmentCreationPrimitive* m_currentSegment = nullptr;

    //параметры объекта "Отрезок"
    QLineEdit* m_startXEdit;
    QLineEdit* m_startYEdit;
    QLineEdit* m_endXEdit;
    QLineEdit* m_endYEdit;
};
