//DxfExporter.cpp - реализация экспорта сцены в формат DXF (AC1015 / AutoCAD 2000)

#include "DxfExporter.h"
#include "Scene.h"
#include "BasePrimitive.h"
#include "PointPrimitive.h"
#include "SegmentPrimitive.h"
#include "CirclePrimitive.h"
#include "ArcPrimitive.h"
#include "EllipsePrimitive.h"
#include "PolygonPrimitive.h"
#include "SplinePrimitive.h"
#include "RectanglePrimitive.h"
#include "../model/primitives/dimensions/BaseDimensionPrimitive.h"
#include "../model/primitives/dimensions/LinearDimensionPrimitive.h"
#include "../model/primitives/dimensions/RadialDimensionPrimitive.h"
#include "../model/primitives/dimensions/AngularDimensionPrimitive.h"
#include "EnumManager.h"
#include "LineStyleManager.h"

#include <QFile>
#include <QTextStream>
#include <QSet>
#include <QHash>
#include <QColor>
#include <QLocale>
#include <QDebug>
#include <cmath>
#include <algorithm>

static int getAutoCadColorIndex(const QColor& color) {
    // Точные совпадения для основных цветов
    if (color == Qt::white) return 7;
    if (color == Qt::red) return 1;
    if (color == Qt::yellow) return 2;
    if (color == Qt::green) return 3;
    if (color == Qt::cyan) return 4;
    if (color == Qt::blue) return 5;
    if (color == Qt::magenta) return 6;
    if (color == Qt::black) return 0; // BYBLOCK
    
    // Простая палитра для поиска ближайшего цвета (основные 15 цветов AutoCAD)
    struct AcColor { int r, g, b, index; };
    static const AcColor acPalette[] = {
        {255, 0, 0, 1}, {255, 255, 0, 2}, {0, 255, 0, 3}, {0, 255, 255, 4},
        {0, 0, 255, 5}, {255, 0, 255, 6}, {255, 255, 255, 7}, {128, 128, 128, 8},
        {192, 192, 192, 9}, {255, 0, 0, 10}, {255, 127, 127, 11},
        {165, 0, 0, 12}, {165, 82, 82, 13}, {127, 0, 0, 14}, {127, 63, 63, 15},
        {255, 63, 0, 20}, {255, 159, 127, 21}, {255, 127, 0, 30}, {255, 191, 127, 31},
        {255, 191, 0, 40}, {255, 223, 127, 41}, {255, 255, 0, 50}, {255, 255, 127, 51},
        {127, 255, 0, 60}, {191, 255, 127, 61}, {0, 255, 0, 80}, {127, 255, 127, 81},
        {0, 255, 127, 100}, {127, 255, 191, 101}, {0, 255, 255, 140}, {127, 255, 255, 141},
        {0, 127, 255, 160}, {127, 191, 255, 161}, {0, 0, 255, 180}, {127, 127, 255, 181},
        {127, 0, 255, 200}, {191, 127, 255, 201}, {255, 0, 255, 220}, {255, 127, 255, 221},
        {255, 0, 127, 240}, {255, 127, 191, 241}, {51, 51, 51, 250}, {91, 91, 91, 251},
        {132, 132, 132, 252}, {173, 173, 173, 253}, {214, 214, 214, 254}, {255, 255, 255, 255}
    };

    int bestIndex = 7;
    int minDist = 255 * 255 * 3;
    for (const auto& c : acPalette) {
        int dr = color.red() - c.r;
        int dg = color.green() - c.g;
        int db = color.blue() - c.b;
        int dist = dr*dr + dg*dg + db*db;
        if (dist < minDist) {
            minDist = dist;
            bestIndex = c.index;
        }
    }
    return bestIndex;
}

// Вспомогательная функция для формирования 24-битного значения TrueColor
static int getTrueColor24Bit(const QColor& color) {
    return (color.red() << 16) | (color.green() << 8) | color.blue();
}

static QString getDxfLineType(int typeId) {
    switch (static_cast<LineType>(typeId)) {
    case LineType::SolidMain:     return "CONTINUOUS";
    case LineType::SolidThin:     return "CONTINUOUS";
    case LineType::SolidWave:     return "CONTINUOUS"; // Геометрия волны экспортируется как POLYLINE
    case LineType::SolidKink:     return "CONTINUOUS"; // Геометрия излома экспортируется как POLYLINE
    case LineType::Dashed:        return "DASHED";
    case LineType::DashDotThick:  return "DASHDOT";
    case LineType::DashDotThin:   return "DASHDOT";
    case LineType::DashDotDot:    return "DIVIDE";
    default:                      return "CONTINUOUS";
    }
}

// Определение толщины линии в DXF (код 370, значение в сотых долях мм)
static int getDxfLineWeight(int typeId) {
    switch (static_cast<LineType>(typeId)) {
    case LineType::SolidMain:
    case LineType::DashDotThick:
        return 50;  // 0.50 мм — толстая линия
    default:
        return 25;  // 0.25 мм — тонкая линия
    }
}

// Проверка, является ли тип линии специальным (волна/зигзаг), требующим геометрической аппроксимации
static bool isSpecialLineType(int typeId) {
    LineType lt = static_cast<LineType>(typeId);
    return lt == LineType::SolidWave || lt == LineType::SolidKink;
}

// ============================================================
// Генерация точек волнистой линии для отрезка (start -> end)
// ============================================================
static QVector<QPointF> generateWavePoints(const QPointF& start, const QPointF& end) {
    const auto& lsm = LineStyleManager::instance();
    double amplitude = lsm.getWaveAmplitude();
    double period = lsm.getWavePeriod();

    QVector<QPointF> pts;
    double dx = end.x() - start.x();
    double dy = end.y() - start.y();
    double length = std::sqrt(dx*dx + dy*dy);
    if (length < 1e-6) { pts.append(start); pts.append(end); return pts; }

    double px = -dy / length; // Перпендикуляр
    double py =  dx / length;

    double step = std::max(1.0, period / 10.0);
    for (double i = 0; i <= length; i += step) {
        double t = i / length;
        double offset = amplitude * std::sin(i * 2.0 * M_PI / period);
        double x = start.x() + dx * t + px * offset;
        double y = start.y() + dy * t + py * offset;
        pts.append(QPointF(x, y));
    }
    // Убеждаемся, что конечная точка включена
    if (pts.isEmpty() || (pts.last() - end).manhattanLength() > 1e-6)
        pts.append(end);
    return pts;
}

// ============================================================
// Генерация точек линии с изломами (зигзаг) для отрезка
// ============================================================
static QVector<QPointF> generateZigzagPoints(const QPointF& start, const QPointF& end) {
    const auto& lsm = LineStyleManager::instance();
    double amplitude = lsm.getKinkAmplitude();
    double kinkLen = lsm.getKinkLength();
    double straightLen = lsm.getKinkStraight();
    double period = 2 * kinkLen + straightLen;

    QVector<QPointF> pts;
    double dx = end.x() - start.x();
    double dy = end.y() - start.y();
    double length = std::sqrt(dx*dx + dy*dy);
    if (length < 1e-6) { pts.append(start); pts.append(end); return pts; }

    double px = -dy / length;
    double py =  dx / length;

    pts.append(start);
    double currentPos = 0;
    while (currentPos < length) {
        // 1. Точка вниз
        double t1 = (currentPos + kinkLen / 2.0) / length;
        if (t1 > 1.0) t1 = 1.0;
        pts.append(QPointF(start.x() + dx * t1 - px * amplitude,
                           start.y() + dy * t1 - py * amplitude));
        // 2. Точка вверх
        double t2 = (currentPos + kinkLen) / length;
        if (t2 > 1.0) { pts.append(end); break; }
        pts.append(QPointF(start.x() + dx * t2 + px * amplitude,
                           start.y() + dy * t2 + py * amplitude));
        // 3. Возврат на линию
        double t3 = (currentPos + kinkLen + kinkLen / 2.0) / length;
        if (t3 > 1.0) t3 = 1.0;
        pts.append(QPointF(start.x() + dx * t3, start.y() + dy * t3));
        // 4. Конец прямого участка
        currentPos += period;
        double t4 = currentPos / length;
        if (t4 > 1.0) t4 = 1.0;
        pts.append(QPointF(start.x() + dx * t4, start.y() + dy * t4));
    }
    if ((pts.last() - end).manhattanLength() > 1e-6)
        pts.append(end);
    return pts;
}

// ============================================================
// Генерация точек волнистой/зигзаг-линии для окружности/эллипса (замкнутая)
// ============================================================
static QVector<QPointF> generateWaveEllipsePoints(const QPointF& center, double rx, double ry, bool isWave) {
    const auto& lsm = LineStyleManager::instance();
    double amplitude = isWave ? lsm.getWaveAmplitude() : lsm.getKinkAmplitude();
    double period = isWave ? lsm.getWavePeriod() : (2.0 * lsm.getKinkLength() + lsm.getKinkStraight());

    double perimeter = 2.0 * M_PI * std::sqrt((rx*rx + ry*ry) / 2.0);
    double cycles = std::round(perimeter / period);
    if (cycles < 1.0) cycles = 1.0;

    int stepsPerPeriod = 40;
    int totalSteps = static_cast<int>(cycles * stepsPerPeriod);

    QVector<QPointF> pts;
    pts.reserve(totalSteps + 2);

    for (int i = 0; i <= totalSteps; ++i) {
        double t = static_cast<double>(i) / totalSteps;
        double angle = t * 2.0 * M_PI;
        double offset = 0.0;

        if (isWave) {
            offset = amplitude * std::sin(angle * cycles);
        } else {
            // Зигзаг (треугольная волна)
            double phase = t * cycles;
            phase -= std::floor(phase);
            if (phase < 0.25) offset = amplitude * (phase / 0.25);
            else if (phase < 0.75) offset = amplitude * (1.0 - (phase - 0.25) / 0.25);
            else offset = -amplitude * (1.0 - (phase - 0.75) / 0.25);
        }

        double rX = rx + offset;
        double rY = ry + offset;
        double x = center.x() + rX * std::cos(angle);
        double y = center.y() + rY * std::sin(angle);
        pts.append(QPointF(x, y));
    }
    return pts;
}

// ============================================================
// Генерация точек волнистой/зигзаг-линии для дуги
// ============================================================
static QVector<QPointF> generateWaveArcPoints(const QPointF& center, double radius,
                                               double startAngleDeg, double spanAngleDeg, bool isWave) {
    const auto& lsm = LineStyleManager::instance();
    double amplitude = isWave ? lsm.getWaveAmplitude() : lsm.getKinkAmplitude();
    double period = isWave ? lsm.getWavePeriod() : (2.0 * lsm.getKinkLength() + lsm.getKinkStraight());

    double startRad = startAngleDeg * M_PI / 180.0;
    double spanRad = spanAngleDeg * M_PI / 180.0;
    double arcLen = std::abs(radius * spanRad);
    double cycles = arcLen / period;

    int totalSteps = std::max(10, static_cast<int>(cycles * 40));

    QVector<QPointF> pts;
    pts.reserve(totalSteps + 2);

    for (int i = 0; i <= totalSteps; ++i) {
        double t = static_cast<double>(i) / totalSteps;
        double currentAngleRad = startRad + t * spanRad;
        double phase = t * cycles;
        double offset = 0.0;

        if (isWave) {
            offset = amplitude * std::sin(phase * 2.0 * M_PI);
        } else {
            double p = phase - std::floor(phase);
            if (p < 0.25) offset = amplitude * (p / 0.25);
            else if (p < 0.75) offset = amplitude * (1.0 - (p - 0.25) / 0.25);
            else offset = -amplitude * (1.0 - (p - 0.75) / 0.25);
        }

        double r = radius + offset;
        // Qt экранные координаты: Y-down, углы CCW
        double x = center.x() + r * std::cos(-currentAngleRad);
        double y = center.y() + r * std::sin(-currentAngleRad);
        pts.append(QPointF(x, y));
    }
    return pts;
}

static QString primitiveTypeToken(PrimitiveType type)
{
    switch (type) {
    case PrimitiveType::Segment: return "SEGMENT";
    case PrimitiveType::Circle: return "CIRCLE";
    case PrimitiveType::Arc: return "ARC";
    case PrimitiveType::Rectangle: return "RECTANGLE";
    case PrimitiveType::Ellipse: return "ELLIPSE";
    case PrimitiveType::Polygon: return "POLYGON";
    case PrimitiveType::Spline: return "SPLINE";
    case PrimitiveType::LinearDimension: return "LINEAR_DIMENSION";
    case PrimitiveType::RadialDimension: return "RADIAL_DIMENSION";
    case PrimitiveType::AngularDimension: return "ANGULAR_DIMENSION";
    case PrimitiveType::Point: return "POINT";
    case PrimitiveType::Generic:
    default:
        return "GENERIC";
    }
}

static QString linearModeToken(LinearDimensionMode mode)
{
    switch (mode) {
    case LinearDimensionMode::Horizontal: return "HORIZONTAL";
    case LinearDimensionMode::Vertical: return "VERTICAL";
    case LinearDimensionMode::Aligned:
    default:
        return "ALIGNED";
    }
}

static QString snapTypeToken(SnapType type)
{
    switch (type) {
    case SnapType::Endpoint: return "ENDPOINT";
    case SnapType::Midpoint: return "MIDPOINT";
    case SnapType::Center: return "CENTER";
    case SnapType::Intersection: return "INTERSECTION";
    case SnapType::Perpendicular: return "PERPENDICULAR";
    case SnapType::Tangent: return "TANGENT";
    case SnapType::Quadrant: return "QUADRANT";
    case SnapType::Grid: return "GRID";
    case SnapType::Nearest: return "NEAREST";
    case SnapType::All: return "ALL";
    case SnapType::None:
    default:
        return "NONE";
    }
}

bool DxfExporter::exportSceneToDxf(const Scene& scene, const QString& filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return false;
    }

    QTextStream out(&file);
    out.setLocale(QLocale::c());
    // Обязательно: FixedNotation с высокой точностью, чтобы doubles всегда
    // выводились с десятичной точкой и достаточным количеством знаков.
    // SmartNotation (по умолчанию) может убрать десятичную точку у целых чисел
    // и обрезать значащие цифры, что ломает DXF-парсеры.
    out.setRealNumberNotation(QTextStream::FixedNotation);
    out.setRealNumberPrecision(10);

    // Счётчик уникальных handle'ов для AC1015
    int handleCounter = 1;
    auto nextHandle = [&handleCounter]() -> QString {
        return QString::number(handleCounter++, 16).toUpper();
    };

    auto writeCode = [&out](int code, const auto& value) {
        out << code << "\n" << value << "\n";
    };

    // Получаем текущие параметры штрихов из LineStyleManager
    const auto& lsm = LineStyleManager::instance();
    double dashLen = lsm.getDashLength();   // по умолчанию 10.0
    double dashSpace = lsm.getDashSpace();  // по умолчанию 5.0

    // Соберем уникальные слои
    QSet<QString> layerNames;
    const auto& primitives = scene.getPrimitives();
    for (const auto& prim : primitives) {
        layerNames.insert(prim->getLayerName());
    }
    if (layerNames.isEmpty()) {
        layerNames.insert("0");
    }
    // Всегда включаем слой "0"
    layerNames.insert("0");

    // Pre-allocate handles для таблиц и блоков
    // Резервируем начальные handle'ы для системных объектов
    QString hVportTable = nextHandle();   // 1
    QString hVportEntry = nextHandle();   // 2
    QString hLtypeTable = nextHandle();   // 3
    // handle'ы для LTYPE записей выделяются далее
    QString hLayerTable, hStyleTable, hAppIdTable, hDimStyleTable, hBlockRecordTable;
    // handle'ы для блоков
    QString hModelSpaceBlock, hPaperSpaceBlock;
    QString hModelSpaceBlockEnd, hPaperSpaceBlockEnd;
    QString hModelSpaceBlockRecord, hPaperSpaceBlockRecord;

    // ================= HEADER SECTION =================
    writeCode(0, "SECTION");
    writeCode(2, "HEADER");
    writeCode(9, "$ACADVER");
    writeCode(1, "AC1015");
    writeCode(9, "$HANDSEED");
    // placeholder — обновим потом; пока Ставим большое значение
    writeCode(5, "FFFF");
    // Глобальный масштаб типов линий ($LTSCALE)
    writeCode(9, "$LTSCALE");
    writeCode(40, 1.0);
    writeCode(0, "ENDSEC");

    // ================= TABLES SECTION =================
    writeCode(0, "SECTION");
    writeCode(2, "TABLES");

    // --- VPORT TABLE ---
    writeCode(0, "TABLE");
    writeCode(2, "VPORT");
    writeCode(5, hVportTable);
    writeCode(100, "AcDbSymbolTable");
    writeCode(70, 1);
    writeCode(0, "VPORT");
    writeCode(5, hVportEntry);
    writeCode(100, "AcDbSymbolTableRecord");
    writeCode(100, "AcDbViewportTableRecord");
    writeCode(2, "*ACTIVE");
    writeCode(70, 0);
    writeCode(10, 0.0); writeCode(20, 0.0);
    writeCode(11, 1.0); writeCode(21, 1.0);
    writeCode(12, 0.0); writeCode(22, 0.0);
    writeCode(13, 0.0); writeCode(23, 0.0);
    writeCode(14, 10.0); writeCode(24, 10.0);
    writeCode(15, 10.0); writeCode(25, 10.0);
    writeCode(16, 0.0); writeCode(26, 0.0); writeCode(36, 1.0);
    writeCode(17, 0.0); writeCode(27, 0.0); writeCode(37, 0.0);
    writeCode(40, 1000.0);
    writeCode(41, 1.0);
    writeCode(42, 50.0);
    writeCode(43, 0.0);
    writeCode(44, 0.0);
    writeCode(50, 0.0);
    writeCode(51, 0.0);
    writeCode(71, 0);
    writeCode(72, 100);
    writeCode(73, 1);
    writeCode(74, 3);
    writeCode(75, 0);
    writeCode(76, 1);
    writeCode(77, 0);
    writeCode(78, 0);
    writeCode(0, "ENDTAB");

    // --- LTYPE TABLE ---
    int ltypeCount = 4; // CONTINUOUS, DASHED, DASHDOT, DIVIDE
    hLtypeTable = nextHandle();
    writeCode(0, "TABLE");
    writeCode(2, "LTYPE");
    writeCode(5, hLtypeTable);
    writeCode(100, "AcDbSymbolTable");
    writeCode(70, ltypeCount);

    auto writeLType = [&writeCode, &nextHandle](const QString& name, const QString& desc) -> QString {
        QString h = nextHandle();
        writeCode(0, "LTYPE");
        writeCode(5, h);
        writeCode(100, "AcDbSymbolTableRecord");
        writeCode(100, "AcDbLinetypeTableRecord");
        writeCode(2, name);
        writeCode(70, 0);
        writeCode(3, desc);
        writeCode(72, 65);  // alignment 'A'
        return h;
    };

    // CONTINUOUS
    writeLType("CONTINUOUS", "Solid line");
    writeCode(73, 0); writeCode(40, 0.0);

    // DASHED: dash, space
    {
        double d = dashLen;
        double s = dashSpace;
        double total = d + s;
        writeLType("DASHED", "Dashed __ __ __ __");
        writeCode(73, 2);
        writeCode(40, total);
        writeCode(49, d);
        writeCode(74, 0);
        writeCode(49, -s);
        writeCode(74, 0);
    }

    // DASHDOT: dash, space, dot, space
    {
        double d = dashLen;
        double s = dashSpace / 2.0;
        double total = d + s + 0.0 + s;
        writeLType("DASHDOT", "Dash dot __ . __ . __");
        writeCode(73, 4);
        writeCode(40, total);
        writeCode(49, d);
        writeCode(74, 0);
        writeCode(49, -s);
        writeCode(74, 0);
        writeCode(49, 0.0);   // dot
        writeCode(74, 0);
        writeCode(49, -s);
        writeCode(74, 0);
    }

    // DIVIDE: dash, space, dot, space, dot, space
    {
        double d = dashLen;
        double s = dashSpace / 2.0;
        double total = d + s + 0.0 + s + 0.0 + s;
        writeLType("DIVIDE", "Dash dot dot __ . . __ . . __");
        writeCode(73, 6);
        writeCode(40, total);
        writeCode(49, d);
        writeCode(74, 0);
        writeCode(49, -s);
        writeCode(74, 0);
        writeCode(49, 0.0);   // dot 1
        writeCode(74, 0);
        writeCode(49, -s);
        writeCode(74, 0);
        writeCode(49, 0.0);   // dot 2
        writeCode(74, 0);
        writeCode(49, -s);
        writeCode(74, 0);
    }

    writeCode(0, "ENDTAB");

    // --- LAYER TABLE ---
    hLayerTable = nextHandle();
    writeCode(0, "TABLE");
    writeCode(2, "LAYER");
    writeCode(5, hLayerTable);
    writeCode(100, "AcDbSymbolTable");
    writeCode(70, layerNames.size());
    for (const QString& layerName : layerNames) {
        QString h = nextHandle();
        writeCode(0, "LAYER");
        writeCode(5, h);
        writeCode(100, "AcDbSymbolTableRecord");
        writeCode(100, "AcDbLayerTableRecord");
        writeCode(2, layerName);
        writeCode(70, 0);
        writeCode(62, 7);
        writeCode(6, "CONTINUOUS");
        writeCode(370, 25); // Default lineweight
    }
    writeCode(0, "ENDTAB");

    // --- STYLE TABLE ---
    hStyleTable = nextHandle();
    writeCode(0, "TABLE");
    writeCode(2, "STYLE");
    writeCode(5, hStyleTable);
    writeCode(100, "AcDbSymbolTable");
    writeCode(70, 1);
    {
        QString h = nextHandle();
        writeCode(0, "STYLE");
        writeCode(5, h);
        writeCode(100, "AcDbSymbolTableRecord");
        writeCode(100, "AcDbTextStyleTableRecord");
        writeCode(2, "STANDARD");
        writeCode(70, 0);
        writeCode(40, 0.0);
        writeCode(41, 1.0);
        writeCode(50, 0.0);
        writeCode(71, 0);
        writeCode(42, 2.5);
        writeCode(3, "txt");
        writeCode(4, "");
    }
    writeCode(0, "ENDTAB");

    // --- APPID TABLE ---
    hAppIdTable = nextHandle();
    writeCode(0, "TABLE");
    writeCode(2, "APPID");
    writeCode(5, hAppIdTable);
    writeCode(100, "AcDbSymbolTable");
    writeCode(70, 1);
    {
        QString h = nextHandle();
        writeCode(0, "APPID");
        writeCode(5, h);
        writeCode(100, "AcDbSymbolTableRecord");
        writeCode(100, "AcDbRegAppTableRecord");
        writeCode(2, "ACAD");
        writeCode(70, 0);
    }
    writeCode(0, "ENDTAB");

    // --- DIMSTYLE TABLE ---
    hDimStyleTable = nextHandle();
    writeCode(0, "TABLE");
    writeCode(2, "DIMSTYLE");
    writeCode(5, hDimStyleTable);
    writeCode(100, "AcDbSymbolTable");
    writeCode(70, 1);
    writeCode(100, "AcDbDimStyleTable");
    {
        QString h = nextHandle();
        writeCode(0, "DIMSTYLE");
        writeCode(5, h);
        writeCode(100, "AcDbSymbolTableRecord");
        writeCode(100, "AcDbDimStyleTableRecord");
        writeCode(2, "STANDARD");
        writeCode(70, 0);
    }
    writeCode(0, "ENDTAB");

    // --- BLOCK_RECORD TABLE ---
    hBlockRecordTable = nextHandle();
    hModelSpaceBlockRecord = nextHandle();
    hPaperSpaceBlockRecord = nextHandle();
    writeCode(0, "TABLE");
    writeCode(2, "BLOCK_RECORD");
    writeCode(5, hBlockRecordTable);
    writeCode(100, "AcDbSymbolTable");
    writeCode(70, 2);

    writeCode(0, "BLOCK_RECORD");
    writeCode(5, hModelSpaceBlockRecord);
    writeCode(100, "AcDbSymbolTableRecord");
    writeCode(100, "AcDbBlockTableRecord");
    writeCode(2, "*Model_Space");

    writeCode(0, "BLOCK_RECORD");
    writeCode(5, hPaperSpaceBlockRecord);
    writeCode(100, "AcDbSymbolTableRecord");
    writeCode(100, "AcDbBlockTableRecord");
    writeCode(2, "*Paper_Space");

    writeCode(0, "ENDTAB");

    writeCode(0, "ENDSEC");

    // ================= BLOCKS SECTION =================
    writeCode(0, "SECTION");
    writeCode(2, "BLOCKS");

    // *Model_Space
    hModelSpaceBlock = nextHandle();
    writeCode(0, "BLOCK");
    writeCode(5, hModelSpaceBlock);
    writeCode(100, "AcDbEntity");
    writeCode(8, "0");
    writeCode(100, "AcDbBlockBegin");
    writeCode(2, "*Model_Space");
    writeCode(70, 0);
    writeCode(10, 0.0); writeCode(20, 0.0); writeCode(30, 0.0);
    writeCode(3, "*Model_Space");
    writeCode(1, "");
    hModelSpaceBlockEnd = nextHandle();
    writeCode(0, "ENDBLK");
    writeCode(5, hModelSpaceBlockEnd);
    writeCode(100, "AcDbEntity");
    writeCode(8, "0");
    writeCode(100, "AcDbBlockEnd");

    // *Paper_Space
    hPaperSpaceBlock = nextHandle();
    writeCode(0, "BLOCK");
    writeCode(5, hPaperSpaceBlock);
    writeCode(100, "AcDbEntity");
    writeCode(8, "0");
    writeCode(100, "AcDbBlockBegin");
    writeCode(2, "*Paper_Space");
    writeCode(70, 0);
    writeCode(10, 0.0); writeCode(20, 0.0); writeCode(30, 0.0);
    writeCode(3, "*Paper_Space");
    writeCode(1, "");
    hPaperSpaceBlockEnd = nextHandle();
    writeCode(0, "ENDBLK");
    writeCode(5, hPaperSpaceBlockEnd);
    writeCode(100, "AcDbEntity");
    writeCode(8, "0");
    writeCode(100, "AcDbBlockEnd");

    writeCode(0, "ENDSEC");

    // ================= ENTITIES SECTION =================
    writeCode(0, "SECTION");
    writeCode(2, "ENTITIES");

    QHash<const BasePrimitive*, int> clarusIds;
    int nextClarusId = 1;
    for (const auto& prim : primitives) {
        clarusIds.insert(prim.get(), nextClarusId++);
    }

    for (const auto& prim : primitives) {
        
        // Общие свойства для AC1015: handle + AcDbEntity subclass + свойства
        auto writeCommonProperties = [&writeCode, &nextHandle, &prim](const QString& subclassMarker) {
            writeCode(5, nextHandle());
            writeCode(100, "AcDbEntity");
            writeCode(8, prim->getLayerName());
            QString lt = getDxfLineType(prim->getLineType());
            if (lt != "CONTINUOUS") {
                writeCode(6, lt);
            }
            writeCode(62, getAutoCadColorIndex(prim->getColor()));
            writeCode(420, getTrueColor24Bit(prim->getColor()));
            // Толщина линии (код 370, в сотых долях мм)
            writeCode(370, getDxfLineWeight(prim->getLineType()));
            writeCode(100, subclassMarker);
        };

        auto writeXData = [&writeCode, &prim]() {
            writeCode(999, QString("CLARUSCAD_LTYPE:%1").arg(static_cast<int>(prim->getLineType())));
        };

        auto writeClarusEntityId = [&writeCode, &prim, &clarusIds]() {
            writeCode(999, QString("CLARUSCAD_ID:%1").arg(clarusIds.value(prim.get(), -1)));
            writeCode(999, QString("CLARUSCAD_PRIMITIVE:%1").arg(primitiveTypeToken(prim->getType())));
        };

        auto writeDimensionStyleMeta = [&writeCode](const BaseDimensionPrimitive* dim) {
            const DimensionStyle style = dim->getStyle();
            writeCode(999, QString("CLARUSCAD_DIM_FONT:%1").arg(style.fontFamily));
            writeCode(999, QString("CLARUSCAD_DIM_TEXT_HEIGHT:%1").arg(style.textHeight, 0, 'f', 10));
            writeCode(999, QString("CLARUSCAD_DIM_TEXT_GAP:%1").arg(style.textGap, 0, 'f', 10));
            writeCode(999, QString("CLARUSCAD_DIM_TEXT_ALONG:%1").arg(style.textAlongLineOffset, 0, 'f', 10));
            writeCode(999, QString("CLARUSCAD_DIM_ARROW_SIZE:%1").arg(style.arrowSize, 0, 'f', 10));
            writeCode(999, QString("CLARUSCAD_DIM_ARROW_TYPE:%1").arg(static_cast<int>(style.arrowType)));
            writeCode(999, QString("CLARUSCAD_DIM_ARROW_FILLED:%1").arg(style.arrowFilled ? 1 : 0));
            writeCode(999, QString("CLARUSCAD_DIM_EXT_OFFSET:%1").arg(style.extensionLineOffset, 0, 'f', 10));
            writeCode(999, QString("CLARUSCAD_DIM_EXT_EXTEND:%1").arg(style.extensionLineExtend, 0, 'f', 10));
            writeCode(999, QString("CLARUSCAD_DIM_LINE_EXT:%1").arg(style.dimensionLineExtension, 0, 'f', 10));
            writeCode(999, QString("CLARUSCAD_DIM_EXT_LTYPE:%1").arg(style.extensionLineTypeId));
            writeCode(999, QString("CLARUSCAD_DIM_LINE_LTYPE:%1").arg(style.dimensionLineTypeId));
            writeCode(999, QString("CLARUSCAD_DIM_TEXT_COLOR:%1").arg(style.textColor.rgba()));
            writeCode(999, QString("CLARUSCAD_DIM_EXT_COLOR:%1").arg(style.extensionLineColor.rgba()));
            writeCode(999, QString("CLARUSCAD_DIM_LINE_COLOR:%1").arg(style.dimensionLineColor.rgba()));
            writeCode(999, QString("CLARUSCAD_DIM_CUSTOM_TEXT:%1").arg(dim->getCustomText()));
            writeCode(999, QString("CLARUSCAD_DIM_HAS_CUSTOM_TEXT_POS:%1").arg(dim->hasCustomTextPosition() ? 1 : 0));
            if (dim->hasCustomTextPosition()) {
                const QPointF pos = dim->getCustomTextPosition();
                writeCode(999, QString("CLARUSCAD_DIM_TEXT_POS_X:%1").arg(pos.x(), 0, 'f', 10));
                writeCode(999, QString("CLARUSCAD_DIM_TEXT_POS_Y:%1").arg(pos.y(), 0, 'f', 10));
            }
        };

        auto writeLinearAttachmentMeta =
            [&writeCode, &clarusIds](const QString& prefix, const LinearDimensionPrimitive::Attachment& attachment) {
                writeCode(999, QString("CLARUSCAD_%1_SNAP:%2").arg(prefix, snapTypeToken(attachment.snapType)));
                writeCode(999, QString("CLARUSCAD_%1_INDEX:%2").arg(prefix).arg(attachment.index));
                writeCode(999, QString("CLARUSCAD_%1_PARAM:%2").arg(prefix).arg(attachment.param, 0, 'f', 10));
                writeCode(999, QString("CLARUSCAD_%1_FALLBACK_X:%2").arg(prefix).arg(attachment.fallback.x(), 0, 'f', 10));
                writeCode(999, QString("CLARUSCAD_%1_FALLBACK_Y:%2").arg(prefix).arg(attachment.fallback.y(), 0, 'f', 10));
                if (attachment.source) {
                    writeCode(999, QString("CLARUSCAD_%1_SOURCE:%2").arg(prefix).arg(clarusIds.value(attachment.source, -1)));
                    writeCode(999, QString("CLARUSCAD_%1_SOURCE_TYPE:%2").arg(prefix, primitiveTypeToken(attachment.source->getType())));
                }
            };

        // Лямбда для записи POLYLINE из набора точек (AC1015)
        auto writePolyline = [&writeCode, &nextHandle, &prim, &writeXData, &writeClarusEntityId](
                                  const QVector<QPointF>& pts, bool closed) {
            writeCode(0, "POLYLINE");
            writeCode(5, nextHandle());
            writeCode(100, "AcDbEntity");
            writeCode(8, prim->getLayerName());
            QString lt = getDxfLineType(prim->getLineType());
            if (lt != "CONTINUOUS") {
                writeCode(6, lt);
            }
            writeCode(62, getAutoCadColorIndex(prim->getColor()));
            writeCode(420, getTrueColor24Bit(prim->getColor()));
            writeCode(370, getDxfLineWeight(prim->getLineType()));
            writeCode(100, "AcDb2dPolyline");
            writeCode(66, 1);
            writeCode(70, closed ? 1 : 0);
            writeCode(10, 0.0);
            writeCode(20, 0.0);
            writeCode(30, 0.0);
            writeXData();
            writeClarusEntityId();
            for (const auto& pt : pts) {
                writeCode(0, "VERTEX");
                writeCode(5, nextHandle());
                writeCode(100, "AcDbEntity");
                writeCode(8, prim->getLayerName());
                writeCode(100, "AcDbVertex");
                writeCode(100, "AcDb2dVertex");
                writeCode(10, pt.x());
                writeCode(20, pt.y());
                writeCode(30, 0.0);
            }
            writeCode(0, "SEQEND");
            writeCode(5, nextHandle());
            writeCode(100, "AcDbEntity");
            writeCode(8, prim->getLayerName());
        };

        PrimitiveType type = prim->getType();
        int lineTypeId = prim->getLineType();
        bool isSpecial = isSpecialLineType(lineTypeId);
        bool isWave = (static_cast<LineType>(lineTypeId) == LineType::SolidWave);

        if (type == PrimitiveType::Segment) {
            auto* segment = dynamic_cast<SegmentPrimitive*>(prim.get());
            if (segment) {
                QPointF start(segment->getStart().getX(), segment->getStart().getY());
                QPointF end(segment->getEnd().getX(), segment->getEnd().getY());

                if (isSpecial) {
                    // Волна или зигзаг — экспортируем как POLYLINE с аппроксимацией формы
                    QVector<QPointF> pts = isWave ? generateWavePoints(start, end)
                                                  : generateZigzagPoints(start, end);
                    writePolyline(pts, false);
                } else {
                    writeCode(0, "LINE");
                    writeCommonProperties("AcDbLine");
                    writeCode(10, start.x());
                    writeCode(20, start.y());
                    writeCode(30, 0.0);
                    writeCode(11, end.x());
                    writeCode(21, end.y());
                    writeCode(31, 0.0);
                    writeXData();
                    writeClarusEntityId();
                }
            }
        }
        else if (type == PrimitiveType::Circle) {
            auto* circle = dynamic_cast<CirclePrimitive*>(prim.get());
            if (circle) {
                if (isSpecial) {
                    QPointF center(circle->getCenter().getX(), circle->getCenter().getY());
                    double r = circle->getRadius();
                    QVector<QPointF> pts = generateWaveEllipsePoints(center, r, r, isWave);
                    writePolyline(pts, true);
                } else {
                    writeCode(0, "CIRCLE");
                    writeCommonProperties("AcDbCircle");
                    writeCode(10, circle->getCenter().getX());
                    writeCode(20, circle->getCenter().getY());
                    writeCode(30, 0.0);
                    writeCode(40, circle->getRadius());
                    writeXData();
                    writeClarusEntityId();
                }
            }
        }
        else if (type == PrimitiveType::Arc) {
            auto* arc = dynamic_cast<ArcPrimitive*>(prim.get());
            if (arc) {
                if (isSpecial) {
                    QPointF center(arc->getCenter().getX(), arc->getCenter().getY());
                    QVector<QPointF> pts = generateWaveArcPoints(center, arc->getRadius(),
                                                                  arc->getStartAngle(), arc->getSpanAngle(), isWave);
                    writePolyline(pts, false);
                } else {
                    writeCode(0, "ARC");
                    // ARC в AC1015: AcDbEntity -> AcDbCircle -> AcDbArc
                    // Qt использует Y-вниз, DXF — Y-вверх.
                    // Углы Qt (CCW в экранных координатах) = CW в мат. координатах.
                    // Для корректного отображения: DXF_angle = -Qt_angle
                    // DXF ARC всегда идёт CCW от startAngle до endAngle.
                    writeCode(5, nextHandle());
                    writeCode(100, "AcDbEntity");
                    writeCode(8, prim->getLayerName());
                    QString lt = getDxfLineType(prim->getLineType());
                    if (lt != "CONTINUOUS") {
                        writeCode(6, lt);
                    }
                    writeCode(62, getAutoCadColorIndex(prim->getColor()));
                    writeCode(420, getTrueColor24Bit(prim->getColor()));
                    writeCode(370, getDxfLineWeight(prim->getLineType()));
                    writeCode(100, "AcDbCircle");
                    writeCode(10, arc->getCenter().getX());
                    writeCode(20, arc->getCenter().getY());
                    writeCode(30, 0.0);
                    writeCode(40, arc->getRadius());
                    writeCode(100, "AcDbArc");
                    // Конвертация углов Qt -> DXF:
                    // Qt: startAngle, spanAngle (CCW при Y-down = CW при Y-up)
                    // DXF: startAngle, endAngle (CCW при Y-up)
                    // Нужно отразить углы: DXF_start = -QtEnd, DXF_end = -QtStart
                    double qtStart = arc->getStartAngle();
                    double qtEnd = qtStart + arc->getSpanAngle();
                    double dxfStart = -qtEnd;
                    double dxfEnd = -qtStart;
                    // Нормализуем в диапазон [0, 360)
                    auto normalizeAngle = [](double a) -> double {
                        a = std::fmod(a, 360.0);
                        if (a < 0) a += 360.0;
                        return a;
                    };
                    dxfStart = normalizeAngle(dxfStart);
                    dxfEnd = normalizeAngle(dxfEnd);
                    writeCode(50, dxfStart);
                    writeCode(51, dxfEnd);
                    writeXData();
                    writeClarusEntityId();
                }
            }
        }
        else if (type == PrimitiveType::Ellipse) {
            auto* ellipse = dynamic_cast<EllipsePrimitive*>(prim.get());
            if (ellipse) {
                if (isSpecial) {
                    QPointF center(ellipse->getCenter().getX(), ellipse->getCenter().getY());
                    QVector<QPointF> pts = generateWaveEllipsePoints(center, ellipse->getRadiusX(),
                                                                     ellipse->getRadiusY(), isWave);
                    writePolyline(pts, true);
                } else {
                    writeCode(0, "ELLIPSE");
                    writeCommonProperties("AcDbEllipse");
                    
                    double rx = ellipse->getRadiusX();
                    double ry = ellipse->getRadiusY();
                    double majorRadius = std::max(rx, ry);
                    double minorRadius = std::min(rx, ry);
                    double ratio = minorRadius / majorRadius;
                    
                    // Qt: rotate(θ) — стандартная матрица вращения, θ в градусах.
                    // В экранных координатах (Y-down):
                    //   rx-ось → направление (cos(θ), sin(θ))
                    //   ry-ось → направление (-sin(θ), cos(θ))
                    // DXF использует Y-up. Координаты центра НЕ инвертируются,
                    // поэтому весь чертёж зеркально отражён по Y.
                    // Для консистентности: инвертируем ТОЛЬКО dy вектора большой оси.
                    double theta = ellipse->getRotation() * M_PI / 180.0;
                    double dx, dy;
                    if (ry > rx) {
                        // Major axis вдоль ry: screen direction = (-sin(θ), cos(θ))
                        // DXF (Y-flip): (-sin(θ), -cos(θ))
                        dx = -majorRadius * std::sin(theta);
                        dy = -majorRadius * std::cos(theta);
                    } else {
                        // Major axis вдоль rx: screen direction = (cos(θ), sin(θ))
                        // DXF (Y-flip): (cos(θ), -sin(θ))
                        dx = majorRadius * std::cos(theta);
                        dy = -majorRadius * std::sin(theta);
                    }
                    
                    writeCode(10, ellipse->getCenter().getX());
                    writeCode(20, ellipse->getCenter().getY());
                    writeCode(30, 0.0);
                    writeCode(11, dx);
                    writeCode(21, dy);
                    writeCode(31, 0.0);
                    writeCode(40, ratio);
                    writeCode(41, 0.0);
                    writeCode(42, 2.0 * M_PI);
                    writeXData();
                    writeClarusEntityId();
                }
            }
        }
        else if (type == PrimitiveType::Polygon) {
            auto* poly = dynamic_cast<PolygonPrimitive*>(prim.get());
            if (poly) {
                if (isSpecial) {
                    // Для специальных типов — генерируем волну/зигзаг по каждому ребру полигона
                    const auto& verts = poly->getVertices();
                    QVector<QPointF> allPts;
                    for (int i = 0; i < verts.size(); ++i) {
                        QPointF p1 = verts[i];
                        QPointF p2 = verts[(i + 1) % verts.size()];
                        QVector<QPointF> edgePts = isWave ? generateWavePoints(p1, p2)
                                                          : generateZigzagPoints(p1, p2);
                        // Не дублируем переходную точку
                        if (!allPts.isEmpty() && !edgePts.isEmpty())
                            edgePts.removeFirst();
                        allPts.append(edgePts);
                    }
                    writePolyline(allPts, false); // Замкнутость через сами точки
                } else {
                    writeCode(0, "POLYLINE");
                    writeCode(5, nextHandle());
                    writeCode(100, "AcDbEntity");
                    writeCode(8, prim->getLayerName());
                    QString lt = getDxfLineType(prim->getLineType());
                    if (lt != "CONTINUOUS") {
                        writeCode(6, lt);
                    }
                    writeCode(62, getAutoCadColorIndex(prim->getColor()));
                    writeCode(420, getTrueColor24Bit(prim->getColor()));
                    writeCode(370, getDxfLineWeight(prim->getLineType()));
                    writeCode(100, "AcDb2dPolyline");
                    writeCode(66, 1);
                    writeCode(70, 1);
                    writeCode(10, 0.0);
                    writeCode(20, 0.0);
                    writeCode(30, 0.0);
                    writeXData();
                    writeClarusEntityId();
                    
                    const auto& pts = poly->getVertices();
                    for (const auto& pt : pts) {
                        writeCode(0, "VERTEX");
                        writeCode(5, nextHandle());
                        writeCode(100, "AcDbEntity");
                        writeCode(8, prim->getLayerName());
                        writeCode(100, "AcDbVertex");
                        writeCode(100, "AcDb2dVertex");
                        writeCode(10, pt.x());
                        writeCode(20, pt.y());
                        writeCode(30, 0.0);
                    }
                    
                    writeCode(0, "SEQEND");
                    writeCode(5, nextHandle());
                    writeCode(100, "AcDbEntity");
                    writeCode(8, prim->getLayerName());
                }
            }
        }
        else if (type == PrimitiveType::Spline) {
            auto* spline = dynamic_cast<SplinePrimitive*>(prim.get());
            if (spline) {
                if (isSpecial) {
                    // Для сплайна — генерируем волну/зигзаг по каждому сегменту аппроксимации
                    auto splinePts = spline->calculateSplinePoints();
                    QVector<QPointF> allPts;
                    for (int i = 0; i < splinePts.size() - 1; ++i) {
                        QVector<QPointF> segPts = isWave ? generateWavePoints(splinePts[i], splinePts[i+1])
                                                         : generateZigzagPoints(splinePts[i], splinePts[i+1]);
                        if (!allPts.isEmpty() && !segPts.isEmpty())
                            segPts.removeFirst();
                        allPts.append(segPts);
                    }
                    writePolyline(allPts, spline->isClosed());
                } else {
                    writeCode(0, "POLYLINE");
                    writeCode(5, nextHandle());
                    writeCode(100, "AcDbEntity");
                    writeCode(8, prim->getLayerName());
                    QString lt = getDxfLineType(prim->getLineType());
                    if (lt != "CONTINUOUS") {
                        writeCode(6, lt);
                    }
                    writeCode(62, getAutoCadColorIndex(prim->getColor()));
                    writeCode(420, getTrueColor24Bit(prim->getColor()));
                    writeCode(370, getDxfLineWeight(prim->getLineType()));
                    writeCode(100, "AcDb2dPolyline");
                    writeCode(66, 1);
                    writeCode(70, spline->isClosed() ? 1 : 0);
                    writeCode(10, 0.0);
                    writeCode(20, 0.0);
                    writeCode(30, 0.0);
                    writeXData();
                    writeClarusEntityId();
                    
                    auto pts = spline->calculateSplinePoints();
                    for (const auto& pt : pts) {
                        writeCode(0, "VERTEX");
                        writeCode(5, nextHandle());
                        writeCode(100, "AcDbEntity");
                        writeCode(8, prim->getLayerName());
                        writeCode(100, "AcDbVertex");
                        writeCode(100, "AcDb2dVertex");
                        writeCode(10, pt.x());
                        writeCode(20, pt.y());
                        writeCode(30, 0.0);
                    }
                    
                    writeCode(0, "SEQEND");
                    writeCode(5, nextHandle());
                    writeCode(100, "AcDbEntity");
                    writeCode(8, prim->getLayerName());
                }
            }
        }
        else if (type == PrimitiveType::Rectangle) {
            auto* rect = dynamic_cast<RectanglePrimitive*>(prim.get());
            if (rect) {
                double w = rect->getWidth();
                double h = rect->getHeight();
                double rot = rect->getRotation() * M_PI / 180.0;
                double cx = rect->getCenter().getX();
                double cy = rect->getCenter().getY();
                
                QVector<QPointF> corners(4);
                corners[0] = QPointF(-w/2, -h/2);
                corners[1] = QPointF(w/2, -h/2);
                corners[2] = QPointF(w/2, h/2);
                corners[3] = QPointF(-w/2, h/2);
                
                // Повернуть и сместить
                for (int i = 0; i < 4; ++i) {
                    double x = corners[i].x() * std::cos(rot) - corners[i].y() * std::sin(rot) + cx;
                    double y = corners[i].x() * std::sin(rot) + corners[i].y() * std::cos(rot) + cy;
                    corners[i] = QPointF(x, y);
                }

                if (isSpecial) {
                    QVector<QPointF> allPts;
                    for (int i = 0; i < 4; ++i) {
                        QPointF p1 = corners[i];
                        QPointF p2 = corners[(i + 1) % 4];
                        QVector<QPointF> edgePts = isWave ? generateWavePoints(p1, p2)
                                                          : generateZigzagPoints(p1, p2);
                        if (!allPts.isEmpty() && !edgePts.isEmpty())
                            edgePts.removeFirst();
                        allPts.append(edgePts);
                    }
                    writePolyline(allPts, false);
                } else {
                    writeCode(0, "POLYLINE");
                    writeCode(5, nextHandle());
                    writeCode(100, "AcDbEntity");
                    writeCode(8, prim->getLayerName());
                    QString lt = getDxfLineType(prim->getLineType());
                    if (lt != "CONTINUOUS") {
                        writeCode(6, lt);
                    }
                    writeCode(62, getAutoCadColorIndex(prim->getColor()));
                    writeCode(420, getTrueColor24Bit(prim->getColor()));
                    writeCode(370, getDxfLineWeight(prim->getLineType()));
                    writeCode(100, "AcDb2dPolyline");
                    writeCode(66, 1);
                    writeCode(70, 1);
                    writeCode(10, 0.0);
                    writeCode(20, 0.0);
                    writeCode(30, 0.0);
                    writeXData();
                    writeClarusEntityId();
                    
                    for (int i = 0; i < 4; ++i) {
                        writeCode(0, "VERTEX");
                        writeCode(5, nextHandle());
                        writeCode(100, "AcDbEntity");
                        writeCode(8, prim->getLayerName());
                        writeCode(100, "AcDbVertex");
                        writeCode(100, "AcDb2dVertex");
                        writeCode(10, corners[i].x());
                        writeCode(20, corners[i].y());
                        writeCode(30, 0.0);
                    }
                    
                    writeCode(0, "SEQEND");
                    writeCode(5, nextHandle());
                    writeCode(100, "AcDbEntity");
                    writeCode(8, prim->getLayerName());
                }
            }
        }
        else if (type == PrimitiveType::LinearDimension) {
            auto* dim = dynamic_cast<LinearDimensionPrimitive*>(prim.get());
            if (dim) {
                const QPointF start = dim->getStartPoint();
                const QPointF end = dim->getEndPoint();
                const QPointF linePos = dim->getDimensionLinePos();
                const QPointF textAnchor = dim->getTextAnchor();
                const QString blockName = QString("*D%1").arg(clarusIds.value(prim.get(), 0));
                const bool hasCustomText = !dim->getCustomText().isEmpty();

                writeCode(0, "DIMENSION");
                writeCode(5, nextHandle());
                writeCode(100, "AcDbEntity");
                writeCode(8, prim->getLayerName());
                writeCode(62, getAutoCadColorIndex(prim->getColor()));
                writeCode(420, getTrueColor24Bit(prim->getColor()));
                writeCode(370, getDxfLineWeight(prim->getLineType()));
                writeCode(100, "AcDbDimension");
                writeCode(2, blockName);
                writeCode(10, textAnchor.x());
                writeCode(20, textAnchor.y());
                writeCode(30, 0.0);
                writeCode(11, linePos.x());
                writeCode(21, linePos.y());
                writeCode(31, 0.0);
                writeCode(70, dim->getMode() == LinearDimensionMode::Aligned ? 1 : 0);
                if (hasCustomText) {
                    writeCode(1, dim->getCustomText());
                }
                writeCode(3, "STANDARD");
                writeCode(42, dim->getMeasuredValue());
                writeCode(100, "AcDbAlignedDimension");
                writeCode(13, start.x());
                writeCode(23, start.y());
                writeCode(33, 0.0);
                writeCode(14, end.x());
                writeCode(24, end.y());
                writeCode(34, 0.0);
                writeCode(15, linePos.x());
                writeCode(25, linePos.y());
                writeCode(35, 0.0);
                if (dim->getMode() != LinearDimensionMode::Aligned) {
                    const double angleDeg = dim->getMode() == LinearDimensionMode::Vertical ? 90.0 : 0.0;
                    writeCode(50, angleDeg);
                }
                writeXData();
                writeClarusEntityId();
                writeCode(999, "CLARUSCAD_DIM_KIND:LINEAR");
                writeCode(999, QString("CLARUSCAD_DIM_MODE:%1").arg(linearModeToken(dim->getMode())));
                writeCode(999, QString("CLARUSCAD_DIM_LINE_X:%1").arg(linePos.x(), 0, 'f', 10));
                writeCode(999, QString("CLARUSCAD_DIM_LINE_Y:%1").arg(linePos.y(), 0, 'f', 10));
                writeDimensionStyleMeta(dim);
                writeLinearAttachmentMeta("DIM_START", dim->getStartAttachment());
                writeLinearAttachmentMeta("DIM_END", dim->getEndAttachment());
            }
        }
        else if (type == PrimitiveType::RadialDimension) {
            auto* dim = dynamic_cast<RadialDimensionPrimitive*>(prim.get());
            if (dim) {
                const QPointF center = dim->getCenterPoint();
                const QPointF radiusPoint = dim->getRadiusPoint();
                const QPointF linePos = dim->getDimensionLinePos();
                const QPointF textAnchor = dim->getTextAnchor();
                const QString blockName = QString("*D%1").arg(clarusIds.value(prim.get(), 0));
                const bool hasCustomText = !dim->getCustomText().isEmpty();
                const double leaderLength = QLineF(center, linePos).length();

                writeCode(0, "DIMENSION");
                writeCode(5, nextHandle());
                writeCode(100, "AcDbEntity");
                writeCode(8, prim->getLayerName());
                writeCode(62, getAutoCadColorIndex(prim->getColor()));
                writeCode(420, getTrueColor24Bit(prim->getColor()));
                writeCode(370, getDxfLineWeight(prim->getLineType()));
                writeCode(100, "AcDbDimension");
                writeCode(2, blockName);
                writeCode(10, textAnchor.x());
                writeCode(20, textAnchor.y());
                writeCode(30, 0.0);
                writeCode(11, linePos.x());
                writeCode(21, linePos.y());
                writeCode(31, 0.0);
                writeCode(70, dim->isDiameterMode() ? 3 : 4);
                if (hasCustomText) {
                    writeCode(1, dim->getCustomText());
                }
                writeCode(3, "STANDARD");
                writeCode(42, dim->getMeasuredValue());
                writeCode(100, dim->isDiameterMode() ? "AcDbDiametricDimension" : "AcDbRadialDimension");
                writeCode(15, center.x());
                writeCode(25, center.y());
                writeCode(35, 0.0);
                writeCode(16, radiusPoint.x());
                writeCode(26, radiusPoint.y());
                writeCode(36, 0.0);
                writeCode(40, leaderLength);
                writeXData();
                writeClarusEntityId();
                writeCode(999, QString("CLARUSCAD_DIM_KIND:%1").arg(dim->isDiameterMode() ? "DIAMETER" : "RADIUS"));
                writeDimensionStyleMeta(dim);
                writeCode(999, QString("CLARUSCAD_DIM_CENTER_X:%1").arg(center.x(), 0, 'f', 10));
                writeCode(999, QString("CLARUSCAD_DIM_CENTER_Y:%1").arg(center.y(), 0, 'f', 10));
                writeCode(999, QString("CLARUSCAD_DIM_RADIUS_X:%1").arg(radiusPoint.x(), 0, 'f', 10));
                writeCode(999, QString("CLARUSCAD_DIM_RADIUS_Y:%1").arg(radiusPoint.y(), 0, 'f', 10));
                writeCode(999, QString("CLARUSCAD_DIM_LINE_X:%1").arg(linePos.x(), 0, 'f', 10));
                writeCode(999, QString("CLARUSCAD_DIM_LINE_Y:%1").arg(linePos.y(), 0, 'f', 10));
                writeCode(999, QString("CLARUSCAD_DIM_ASSOC_ANGLE:%1").arg(dim->getAssociationAngle(), 0, 'f', 10));
                if (dim->getAssociatedPrimitive()) {
                    writeCode(999, QString("CLARUSCAD_DIM_ASSOC_SOURCE:%1").arg(clarusIds.value(dim->getAssociatedPrimitive(), -1)));
                    writeCode(999, QString("CLARUSCAD_DIM_ASSOC_SOURCE_TYPE:%1").arg(primitiveTypeToken(dim->getAssociatedPrimitive()->getType())));
                }
            }
        }
        else if (type == PrimitiveType::AngularDimension) {
            auto* dim = dynamic_cast<AngularDimensionPrimitive*>(prim.get());
            if (dim) {
                const QPointF center = dim->getCenterPoint();
                const QPointF start = dim->getStartPoint();
                const QPointF end = dim->getEndPoint();
                const QPointF arcPoint = dim->getArcPoint();
                const QPointF textAnchor = dim->getTextAnchor();
                const QString blockName = QString("*D%1").arg(clarusIds.value(prim.get(), 0));
                const bool hasCustomText = !dim->getCustomText().isEmpty();

                writeCode(0, "DIMENSION");
                writeCode(5, nextHandle());
                writeCode(100, "AcDbEntity");
                writeCode(8, prim->getLayerName());
                writeCode(62, getAutoCadColorIndex(prim->getColor()));
                writeCode(420, getTrueColor24Bit(prim->getColor()));
                writeCode(370, getDxfLineWeight(prim->getLineType()));
                writeCode(100, "AcDbDimension");
                writeCode(2, blockName);
                writeCode(10, textAnchor.x());
                writeCode(20, textAnchor.y());
                writeCode(30, 0.0);
                writeCode(11, arcPoint.x());
                writeCode(21, arcPoint.y());
                writeCode(31, 0.0);
                writeCode(70, 5);
                if (hasCustomText) {
                    writeCode(1, dim->getCustomText());
                }
                writeCode(3, "STANDARD");
                writeCode(42, dim->getMeasuredValue());
                writeCode(100, "AcDb3PointAngularDimension");
                writeCode(13, start.x());
                writeCode(23, start.y());
                writeCode(33, 0.0);
                writeCode(14, end.x());
                writeCode(24, end.y());
                writeCode(34, 0.0);
                writeCode(15, center.x());
                writeCode(25, center.y());
                writeCode(35, 0.0);
                writeCode(16, arcPoint.x());
                writeCode(26, arcPoint.y());
                writeCode(36, 0.0);
                writeXData();
                writeClarusEntityId();
                writeCode(999, "CLARUSCAD_DIM_KIND:ANGULAR");
                writeDimensionStyleMeta(dim);
                writeCode(999, QString("CLARUSCAD_DIM_CENTER_X:%1").arg(center.x(), 0, 'f', 10));
                writeCode(999, QString("CLARUSCAD_DIM_CENTER_Y:%1").arg(center.y(), 0, 'f', 10));
                writeCode(999, QString("CLARUSCAD_DIM_START_X:%1").arg(start.x(), 0, 'f', 10));
                writeCode(999, QString("CLARUSCAD_DIM_START_Y:%1").arg(start.y(), 0, 'f', 10));
                writeCode(999, QString("CLARUSCAD_DIM_END_X:%1").arg(end.x(), 0, 'f', 10));
                writeCode(999, QString("CLARUSCAD_DIM_END_Y:%1").arg(end.y(), 0, 'f', 10));
                writeCode(999, QString("CLARUSCAD_DIM_ARC_X:%1").arg(arcPoint.x(), 0, 'f', 10));
                writeCode(999, QString("CLARUSCAD_DIM_ARC_Y:%1").arg(arcPoint.y(), 0, 'f', 10));
                if (dim->getFirstAssociatedPrimitive()) {
                    writeCode(999, QString("CLARUSCAD_DIM_FIRST_SOURCE:%1").arg(clarusIds.value(dim->getFirstAssociatedPrimitive(), -1)));
                    writeCode(999, QString("CLARUSCAD_DIM_FIRST_EDGE:%1").arg(dim->getFirstAssociatedEdgeIndex()));
                }
                if (dim->getSecondAssociatedPrimitive()) {
                    writeCode(999, QString("CLARUSCAD_DIM_SECOND_SOURCE:%1").arg(clarusIds.value(dim->getSecondAssociatedPrimitive(), -1)));
                    writeCode(999, QString("CLARUSCAD_DIM_SECOND_EDGE:%1").arg(dim->getSecondAssociatedEdgeIndex()));
                }
            }
        }
    }

    writeCode(0, "ENDSEC");

    // ================= OBJECTS SECTION (required for AC1015) =================
    writeCode(0, "SECTION");
    writeCode(2, "OBJECTS");
    // Словарь корневой
    QString hRootDict = nextHandle();
    writeCode(0, "DICTIONARY");
    writeCode(5, hRootDict);
    writeCode(100, "AcDbDictionary");
    writeCode(281, 1);
    writeCode(0, "ENDSEC");

    // ================= EOF =================
    writeCode(0, "EOF");

    file.close();
    return true;
}
