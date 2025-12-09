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

//типы линий (согласно ГОСТ 2.303-68)
enum class LineType {
    SolidMain, //сплошная основная (толстая)
    SolidThin, //сплошная тонкая
    SolidWave, //сплошная волнистая (требует спец. реализации, пока сделаем заглушку)
    Dashed, //штриховая
    DashDotThick, //штрихпунктирная утолщенная
    DashDotThin, //штрихпунктирная тонкая
    DashDotDot, //штрихпунктирная с двумя точками
    SolidKink //сплошная тонкая с изломами (тоже требует спец. реализации)
};

//толщины линии
enum class LineWeight {
    Thin, //S/2 или S/3
    Thick //S
};
