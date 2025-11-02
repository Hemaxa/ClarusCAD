//EnumManager - класс глобальных перечислений в программе

#pragma once

//enum class - строго типизированный способ определения перечисления в C++

//типы примитивов
enum class PrimitiveType {
    Generic, //общий
    Point, //точка
    Segment //отрезок
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

//типы линий
enum class LineType {
    Solid, //cплошная (будет использоваться как тонкая)
    SolidThick, //cплошная толстая
    Dashed, //штриховая
    Dotted, //пунктирная
    DashDot, //штрих-пунктирная
    DashDotDot //штрих-две-точки
};
