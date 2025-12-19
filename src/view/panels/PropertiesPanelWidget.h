//PropertiesPanelWidget - панель свойств объекта

#pragma once

#include "BasePanelWidget.h"
#include "PointPrimitive.h"
#include "RectanglePropertiesWidget.h"
#include "ArcPropertiesWidget.h"
#include "RectanglePrimitive.h"
#include "ArcPrimitive.h"
#include "EllipsePropertiesWidget.h"
#include "EllipsePrimitive.h"
#include "PolygonPropertiesWidget.h"
#include "PolygonPrimitive.h"
#include "SplinePropertiesWidget.h"
#include "SplinePrimitive.h"

#include <QColor>
#include <QList>

class QStackedWidget;

class SegmentPropertiesWidget;
class CirclePropertiesWidget;
class CommonPropertiesWidget;

class BasePrimitive;
class SegmentPrimitive;
class CirclePrimitive;

//наслдедуется от базового класса BasePanelWidget
class PropertiesPanelWidget : public BasePanelWidget
{
    Q_OBJECT

public:
    //конструктор
    explicit PropertiesPanelWidget(const QString& title, QWidget* parent = nullptr);

public slots:
    //перегрузка слота showPropertiesFor
    //слот, определяющий параметры какого объекта необходимо показать
    //Теперь принимает СПИСОК
    void showPropertiesFor(const QList<BasePrimitive*>& primitives);

    //слот, показывающий пустые параметры при создании нового объекта (при активации инструмента создания объекта)
    void showPropertiesFor(PrimitiveType type);

    //слот, определяющий какую систему координат использовать
    void setCoordinateSystem(CoordinateSystemType type);

    //слот для обновления цветов иконок при смене темы
    void updateColors();

signals:
    //сигнал, информирующий MainWindow о смене цвета в режиме создания объекта
    void colorChanged(const QColor& color);

    //сигнал, информирующий MainWindow о смене типа линии в режиме создания объекта
    void lineTypeChanged(LineType type);

    //сигналы, информирующие MainWindow о создании или изменении объекта
    void segmentPropertiesApplied(SegmentPrimitive* segment, const PointPrimitive& start, const PointPrimitive& end, const QColor& color, LineType lineType);
    void circlePropertiesApplied(CirclePrimitive* circle, const PointPrimitive& center, double radius, const QColor& color, LineType lineType);
    void rectanglePropertiesApplied(RectanglePrimitive* rect, const PointPrimitive& center, double w, double h, double r, CornerType cornerType, double cornerRadius, const QColor& c, LineType t);
    void arcPropertiesApplied(ArcPrimitive* arc, const PointPrimitive& center, double rad, double start, double span, const QColor& c, LineType t);
    void ellipsePropertiesApplied(EllipsePrimitive* ell, const PointPrimitive& center, double rx, double ry, double rot, const QColor& c, LineType t);
    void polygonPropertiesApplied(PolygonPrimitive* polygon, int sides, PolygonCreationMode type, const QColor& color, LineType lineType);
    void splinePropertiesApplied(SplinePrimitive* spline, bool closed, const QVector<QPointF>& controlPoints, const QColor& color, LineType lineType);
    
    //сигналы для обновления параметров инструментов
    void polygonSidesChanged(int sides);
    void polygonTypeChanged(PolygonCreationMode type);
    void splineClosedChanged(bool closed);
    
    //сигнал для применения общих свойств ко всем выделенным объектам
    void commonPropertiesApplied(const QColor& color, int lineTypeId);

private:
    QStackedWidget* m_stack; //панель виджетов без содержимого
    QWidget* m_emptyWidget; //пустой виджет
    SegmentPropertiesWidget* m_segmentProperties; //виджет свойств объекта "Отрезок"
    CirclePropertiesWidget* m_circleProperties; //виджет свойств объекта "Окружность"
    RectanglePropertiesWidget* m_rectProperties;
    ArcPropertiesWidget* m_arcProperties;
    EllipsePropertiesWidget* m_ellipseProperties;
    PolygonPropertiesWidget* m_polygonProperties;
    SplinePropertiesWidget* m_splineProperties;
    CommonPropertiesWidget* m_commonProperties; //виджет общих свойств для мультивыделения
};
