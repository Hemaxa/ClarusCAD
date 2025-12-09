#pragma once
#include "BasePrimitive.h"
#include "PointPrimitive.h"

class RectanglePrimitive : public BasePrimitive {
public:
    RectanglePrimitive(const PointPrimitive& center, double width, double height, double rotation = 0.0);

    PrimitiveType getType() const override { return PrimitiveType::Rectangle; }
    QString getTypeName() const override { return "Прямоугольник"; }
    QRectF getBoundingBox() const override;

    PointPrimitive getCenter() const { return m_center; }
    void setCenter(const PointPrimitive& c) { m_center = c; }

    double getWidth() const { return m_width; }
    void setWidth(double w) { m_width = w; }

    double getHeight() const { return m_height; }
    void setHeight(double h) { m_height = h; }

    double getRotation() const { return m_rotation; }
    void setRotation(double r) { m_rotation = r; }

    CornerType getCornerType() const { return m_cornerType; }
    void setCornerType(CornerType type) { m_cornerType = type; }

    double getCornerRadius() const { return m_cornerRadius; }
    void setCornerRadius(double r) { m_cornerRadius = r; }

private:
    PointPrimitive m_center;
    double m_width;
    double m_height;
    double m_rotation; // Угол поворота в градусах
    CornerType m_cornerType = CornerType::None;
    double m_cornerRadius = 0.0; // Для фасок и скруглений
};
