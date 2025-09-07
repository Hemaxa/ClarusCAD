#pragma once

#include "BasePrimitive.h"

class Point : public BasePrimitive
{

public:
    Point(double x = 0.0, double y = 0.0);

    double x() const;
    void setX(double x);

    double y() const;
    void setY(double y);

    PrimitiveType getType() const override { return PrimitiveType::Point; }

private:
    double m_x;
    double m_y;
};
