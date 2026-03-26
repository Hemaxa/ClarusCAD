//SplinePrimitive - примитив сплайна (кубический сплайн Безье)
//Параметры: список контрольных точек

#pragma once

#include "BasePrimitive.h"
#include "PointPrimitive.h"
#include <QVector>

class SplinePrimitive : public BasePrimitive
{
public:
    /**
     * @brief Конструктор сплайна (пустой).
     */
    SplinePrimitive();

    /**
     * @brief Конструктор сплайна из контрольных точек.
     */
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
    
    /**
     * @brief Добавить контрольную точку в конец.
     */
    void addControlPoint(const QPointF& point);

    /**
     * @brief Вставить контрольную точку по индексу.
     */
    void insertControlPoint(int index, const QPointF& point);

    /**
     * @brief Удалить контрольную точку по индексу.
     */
    void removeControlPoint(int index);

    /**
     * @brief Переместить контрольную точку.
     */
    void moveControlPoint(int index, const QPointF& newPos);
    
    int getControlPointCount() const { return m_controlPoints.size(); }

    /**
     * @brief Проверить, замкнут ли сплайн.
     */
    bool isClosed() const { return m_closed; }

    /**
     * @brief Установить замкнутость сплайна.
     */
    void setClosed(bool closed) { m_closed = closed; }

    // Вычисление точек сплайна для отрисовки (используется и для экспорта DXF)
    QVector<QPointF> calculateSplinePoints(int segmentsPerCurve = 20) const;

private:
    // Кубический интерполяционный сплайн (Catmull-Rom)
    QPointF catmullRom(const QPointF& p0, const QPointF& p1, 
                       const QPointF& p2, const QPointF& p3, double t) const;

    QVector<QPointF> m_controlPoints;
    bool m_closed = false;
};
