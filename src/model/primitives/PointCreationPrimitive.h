#pragma once

#include "BasePrimitive.h"

class PointCreationPrimitive : public BasePrimitive
{

public:
    PointCreationPrimitive(double x = 0.0, double y = 0.0);

    PrimitiveType getType() const override { return PrimitiveType::Point; }

    double x() const;
    void setX(double x);

    double y() const;
    void setY(double y);

private:
    double m_x;
    double m_y;
};
