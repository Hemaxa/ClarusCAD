//SplinePrimitive - примитив сплайна (кубический сплайн Безье)
//Параметры: список контрольных точек

#pragma once

#include "BasePrimitive.h"
#include "PointPrimitive.h"
#include <QVector>

class SplinePrimitive : public BasePrimitive
{
public:
    SplinePrimitive();
    explicit SplinePrimitive(const QVector<QPointF>& controlPoints);

    // Переопределение типа
    PrimitiveType getType() const override { return PrimitiveType::Spline; }
    QString getTypeName() const override { return "Сплайн"; }

    // Smart Model методы
    void draw(QPainter& painter, bool isSelected) const override;
    QRectF getBoundingBox() const override;
    bool hitTest(const QPointF& point, double tolerance) const override;
    bool intersects(const QRectF& rect) const override;
    bool inside(const QRectF& rect) const override;
    QVector<QPointF> getSnapPoints() const override;

    // Управление контрольными точками
    QVector<QPointF> getControlPoints() const { return m_controlPoints; }
    void setControlPoints(const QVector<QPointF>& points) { m_controlPoints = points; }
    
    void addControlPoint(const QPointF& point);
    void insertControlPoint(int index, const QPointF& point);
    void removeControlPoint(int index);
    void moveControlPoint(int index, const QPointF& newPos);
    
    int getControlPointCount() const { return m_controlPoints.size(); }

    // Флаг замкнутости
    bool isClosed() const { return m_closed; }
    void setClosed(bool closed) { m_closed = closed; }

private:
    // Вычисление точек сплайна для отрисовки
    QVector<QPointF> calculateSplinePoints(int segmentsPerCurve = 20) const;
    
    // Кубический интерполяционный сплайн (Catmull-Rom)
    QPointF catmullRom(const QPointF& p0, const QPointF& p1, 
                       const QPointF& p2, const QPointF& p3, double t) const;

    QVector<QPointF> m_controlPoints;
    bool m_closed = false;
};
