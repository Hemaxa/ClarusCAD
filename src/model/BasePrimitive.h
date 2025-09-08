#pragma once

enum class PrimitiveType {
    Generic,
    Point,
    Segment
};

class BasePrimitive
{

public:
    virtual ~BasePrimitive() = default;
    virtual PrimitiveType getType() const { return PrimitiveType::Generic; }
};
