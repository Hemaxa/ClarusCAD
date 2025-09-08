#pragma once

#include "BasePrimitive.h"
#include "PointCreationPrimitive.h"
#include "SegmentCreationPrimitive.h"

#include <vector>
#include <memory>


class Scene
{

public:
    Scene();
    void addPrimitive(std::unique_ptr<BasePrimitive> primitive);
    const std::vector<std::unique_ptr<BasePrimitive>>& getPrimitives() const;

private:
    std::vector<std::unique_ptr<BasePrimitive>> m_primitives;
};
