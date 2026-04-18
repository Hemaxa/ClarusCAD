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
#include "DimensionPropertiesWidget.h"

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
    /**
     * @brief Конструктор.
     */
    explicit PropertiesPanelWidget(const QString& title, QWidget* parent = nullptr);

public slots:
    /**
     * @brief Показать свойства для списка примитивов.
     * @param primitives Список выделенных примитивов.
     */
    void showPropertiesFor(const QList<BasePrimitive*>& primitives);

    /**
     * @brief Показать пустые свойства для нового объекта определенного типа.
     * @param type Тип создаваемого примитива.
     */
    void showPropertiesFor(PrimitiveType type);

    /**
     * @brief Установить текущую систему координат.
     */
    void setCoordinateSystem(CoordinateSystemType type);

    /**
     * @brief Обновить иконки при смене темы.
     */
    void updateColors();

signals:
    /**
     * @brief Сигнал изменения слоя.
     */
    void layerChanged(const QString& name);
    /**
     * @brief Сигнал смены цвета при создании нового объекта.
     */
    void colorChanged(const QColor& color);

    /**
     * @brief Сигнал смены типа линии при создании нового объекта.
     */
    void lineTypeChanged(LineType type);

    // Сигналы, информирующие MainWindow о создании или изменении объекта (проксируются от виджетов свойств)
    void segmentPropertiesApplied(SegmentPrimitive* segment, const PointPrimitive& start, const PointPrimitive& end, const QColor& color, LineType lineType);
    void circlePropertiesApplied(CirclePrimitive* circle, const PointPrimitive& center, double radius, const QColor& color, LineType lineType);
    void rectanglePropertiesApplied(RectanglePrimitive* rect, const PointPrimitive& center, double w, double h, double r, CornerType cornerType, double cornerRadius, const QColor& c, LineType t);
    void arcPropertiesApplied(ArcPrimitive* arc, const PointPrimitive& center, double rad, double start, double span, const QColor& c, LineType t);
    void ellipsePropertiesApplied(EllipsePrimitive* ell, const PointPrimitive& center, double rx, double ry, double rot, const QColor& c, LineType t);
    void polygonPropertiesApplied(PolygonPrimitive* polygon, int sides, PolygonCreationMode type, const QColor& color, LineType lineType);
    void splinePropertiesApplied(SplinePrimitive* spline, bool closed, const QVector<QPointF>& controlPoints, const QColor& color, LineType lineType);
    
    // Сигналы для обновления параметров инструментов
    void polygonSidesChanged(int sides);
    void polygonTypeChanged(PolygonCreationMode type);
    void splineClosedChanged(bool closed);
    void dimensionPropertiesApplied();
    
    /**
     * @brief Сигнал применения общих свойств (цвет, тип линии) для нескольких объектов.
     */
    void commonPropertiesApplied(const QColor& color, int lineTypeId);

private:
    QStackedWidget* m_stack;              ///< Стек виджетов свойств
    QWidget* m_emptyWidget;               ///< Пустой виджет (когда ничего не выбрано)
    SegmentPropertiesWidget* m_segmentProperties;
    CirclePropertiesWidget* m_circleProperties;
    RectanglePropertiesWidget* m_rectProperties;
    ArcPropertiesWidget* m_arcProperties;
    EllipsePropertiesWidget* m_ellipseProperties;
    PolygonPropertiesWidget* m_polygonProperties;
    SplinePropertiesWidget* m_splineProperties;
    DimensionPropertiesWidget* m_dimensionProperties;
    CommonPropertiesWidget* m_commonProperties; ///< Виджет общих свойств для мультивыделения
};
