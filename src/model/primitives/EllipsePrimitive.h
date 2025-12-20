//EllipsePrimitive - модель эллипса

#pragma once
#include "BasePrimitive.h"
#include "PointPrimitive.h"

class EllipsePrimitive : public BasePrimitive {
public:
    /**
     * @brief Конструктор эллипса.
     * @param center Центр эллипса.
     * @param rx Радиус по оси X (до поворота).
     * @param ry Радиус по оси Y (до поворота).
     * @param rotation Угол поворота в градусах.
     */
    EllipsePrimitive(const PointPrimitive& center, double rx, double ry, double rotation = 0.0);

    PrimitiveType getType() const override { return PrimitiveType::Ellipse; }
    QString getTypeName() const override { return "Эллипс"; }

    // --- Smart Model ---
    void draw(QPainter& painter, bool isSelected) const override;
    QRectF getBoundingBox() const override;
    bool hitTest(const QPointF& point, double tolerance) const override;
    bool intersects(const QRectF& rect) const override;
    bool inside(const QRectF& rect) const override;
    QVector<QPointF> getSnapPoints() const override;

    /**
     * @brief Получить центр.
     */
    PointPrimitive getCenter() const { return m_center; }
    void setCenter(const PointPrimitive& c) { m_center = c; }

    /**
     * @brief Получить радиус по X.
     */
    double getRadiusX() const { return m_radiusX; }
    void setRadiusX(double r) { m_radiusX = r; }

    /**
     * @brief Получить радиус по Y.
     */
    double getRadiusY() const { return m_radiusY; }
    void setRadiusY(double r) { m_radiusY = r; }

    /**
     * @brief Получить угол поворота.
     */
    double getRotation() const { return m_rotation; }
    void setRotation(double r) { m_rotation = r; }

private:
    PointPrimitive m_center; ///< Центр эллипса
    double m_radiusX;        ///< Радиус по X
    double m_radiusY;        ///< Радиус по Y
    double m_rotation;       ///< Угол поворота
};
