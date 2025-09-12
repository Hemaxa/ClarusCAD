//PropertiesPanelWidget - панель свойств объекта

#pragma once

#include "BaseDockWidget.h"
#include "PointCreationPrimitive.h"

class QStackedWidget;
class SegmentPropertiesWidget;
class BasePrimitive;

//наслдедуется от базового класса BaseDockWidget
class PropertiesPanelWidget : public BaseDockWidget
{
    Q_OBJECT

public:
    //конструктор
    explicit PropertiesPanelWidget(const QString& title, QWidget* parent = nullptr);

public slots:
    //слот, определяющий параметры какого объекта необходимо показать
    void showPropertiesFor(BasePrimitive* primitive);

    //слот, показывающий пустые параметры при создании нового объекта
    void showPropertiesFor(PrimitiveType type);

signals:
    //сигналы, информирующие MainWindow о создании объекта
    void createSegmentRequested(const PointCreationPrimitive& start, const PointCreationPrimitive& end);

private:
    QStackedWidget* m_stack; //панель виджетов без содержимого
    SegmentPropertiesWidget* m_segmentProperties; //виджет свойств объекта "Отрезок"
    QWidget* m_emptyWidget; //пустой виджет
};
