#pragma once

class Point
{

public:
    Point(double x = 0.0, double y = 0.0);

    double x() const;
    void setX(double x);

    double y() const;
    void setY(double y);

private:
    double m_x;
    double m_y;
};
