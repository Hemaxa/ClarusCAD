//EnumManager - класс глобальных перечислений в программе

#pragma once

//enum class - строго типизированный способ определения перечисления в C++

//типы примитивов
enum class PrimitiveType {
    Generic,
    Point,
    Segment
};

//типы систем координат
enum class CoordinateSystemType {
    Cartesian, //декартовая
    Polar //полярная
};

//единицы измерения углов
enum class AngleUnit {
    Degrees, //градусы
    Radians //радианы
};
