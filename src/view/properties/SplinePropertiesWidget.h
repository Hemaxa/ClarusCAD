//SplinePropertiesWidget - виджет свойств примитива "Сплайн"

#pragma once

#include "BasePropertiesWidget.h"
#include "PointPrimitive.h"

#include <QLabel>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QVector>
#include <QPair>

class QCheckBox;
class QPushButton;
class QLineEdit;
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
                           const QVector<QPointF>& controlPoints,
                           const QColor& color, LineType lineType);
    
    //сигнал для обновления параметра инструмента
    void closedChanged(bool closed);

private slots:
    //слот для обработки нажатия кнопки "Создать"
    void onApplyButtonClicked();
    
    //слот для обновления инструмента при изменении параметра
    void onClosedChanged(bool checked);
    
    //слоты для редактирования точек
    void onDeletePointClicked();

private:
    //реализация виртуального метода из BasePropertiesWidget
    void updateFieldValues() override;
    
    //обновление списка полей контрольных точек
    void rebuildControlPointsUI();
    
    //получение координат из полей
    QVector<QPointF> getControlPointsFromFields() const;

    //указатель на текущий редактируемый объект
    SplinePrimitive* m_currentSpline = nullptr;

    //поля объекта "Сплайн"
    QCheckBox* m_closedCheckBox;   //замкнутый/разомкнутый
    
    //контейнер для полей контрольных точек
    QWidget* m_pointsContainer;
    QVBoxLayout* m_pointsLayout;
    QScrollArea* m_pointsScrollArea;
    
    //пары полей (X, Y) и кнопка удаления для каждой точки
    struct PointRow {
        QLineEdit* xEdit;
        QLineEdit* yEdit;
        QPushButton* deleteBtn;
        QWidget* container;
    };
    QVector<PointRow> m_pointRows;
};
