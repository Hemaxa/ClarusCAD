//BasePrimitive - базовый класс для всех хранилищ данных объектов

#pragma once

//enum class - строго типизированный способ определения перечисления в C++
//используется для идентификации типов примитивов
enum class PrimitiveType {
    Generic,
    Point,
    Segment
};

class BasePrimitive
{

public:
    //виртуальные деструктор и метод получения типа
    virtual ~BasePrimitive() = default;
    virtual PrimitiveType getType() const { return PrimitiveType::Generic; }
};
