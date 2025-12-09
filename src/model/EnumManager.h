#pragma once

// Типы примитивов
enum class PrimitiveType {
    Generic,
    Point,
    Segment,
    Circle,
    Arc,
    Rectangle,
    Ellipse,
    Polygon,
    Spline
};

// Режимы построения окружности
enum class CircleCreationMode {
    CenterRadius,
    CenterDiameter,
    TwoPoints,
    ThreePoints
};

// Режимы построения дуги
enum class ArcCreationMode {
    ThreePoints,        // Три точки (Начало, Промежуточная, Конец)
    CenterStartEnd      // Центр, Начальный угол, Конечный угол
};

// Режимы построения прямоугольника
enum class RectangleCreationMode {
    TwoPoints,          // По диагонали (P1, P2)
    CenterSize,         // Центр и угол (половина диагонали)
    ThreePoints         // Три точки (задает поворот)
};

// Режимы построения эллипса
enum class EllipseCreationMode {
    CenterAxes          // Центр и две полуоси
};

// Типы углов прямоугольника
enum class CornerType {
    None,       // Прямой угол
    Fillet,     // Скругление
    Chamfer     // Фаска
};

// Режимы построения многоугольника
enum class PolygonCreationMode {
    Inscribed,      // Вписанный в окружность
    Circumscribed   // Описанный вокруг окружности
};

enum class CoordinateSystemType {
    Cartesian,
    Polar
};

enum class AngleUnit {
    Degrees,
    Radians
};

enum class LineType {
    SolidMain,
    SolidThin,
    SolidWave,
    Dashed,
    DashDotThick,
    DashDotThin,
    DashDotDot,
    SolidKink
};

enum class LineWeight {
    Thin,
    Thick
};
