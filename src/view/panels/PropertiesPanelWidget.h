//PropertiesPanelWidget - панель свойств объекта

#pragma once

#include "BasePanelWidget.h"
#include "PointPrimitive.h"

#include <QColor>

class QStackedWidget;
class SegmentPropertiesWidget;
class BasePrimitive;
class SegmentPrimitive;

//наслдедуется от базового класса BasePanelWidget
class PropertiesPanelWidget : public BasePanelWidget
{
    Q_OBJECT

public:
    //конструктор
    explicit PropertiesPanelWidget(const QString& title, QWidget* parent = nullptr);

public slots:
    //перегрузка слота showPropertiesFor
    //слот, определяющий параметры какого объекта необходимо показать (уже существующий объект)
    void showPropertiesFor(BasePrimitive* primitive);

    //слот, показывающий пустые параметры при создании нового объекта (при активации инструмента создания объекта)
    void showPropertiesFor(PrimitiveType type);

    //слот, показывающий пусты параметры с сохраненным цветом при создании нового объекта
    void showPropertiesFor(const QColor& color);

signals:
    //сигнал, информирующий MainWindow о смене цвета в режиме создания объекта
    void colorChanged(const QColor& color);

    //сигналы, информирующие MainWindow о создании или изменении объекта
    void segmentPropertiesApplied(SegmentPrimitive* segment, const PointPrimitive& start, const PointPrimitive& end, const QColor& color);

private:
    QStackedWidget* m_stack; //панель виджетов без содержимого
    SegmentPropertiesWidget* m_segmentProperties; //виджет свойств объекта "Отрезок"
    QWidget* m_emptyWidget; //пустой виджет
};
