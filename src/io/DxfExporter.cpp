//DxfExporter.cpp - реализация экспорта сцены в формат DXF

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
#include "EnumManager.h"
#include "LineStyleManager.h"

#include <QFile>
#include <QTextStream>
#include <QSet>
#include <QColor>
#include <QLocale>
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

// Вспомогательная функция для формирования 24-битного значения TrueColor (оставлена на случай импорта)
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

bool DxfExporter::exportSceneToDxf(const Scene& scene, const QString& filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return false;
    }

    QTextStream out(&file);
    out.setLocale(QLocale::c());

    auto writeCode = [&out](int code, const auto& value) {
        out << code << "\n" << value << "\n";
    };

    // Получаем текущие параметры штрихов из LineStyleManager
    const auto& lsm = LineStyleManager::instance();
    double dashLen = lsm.getDashLength();   // по умолчанию 10.0
    double dashSpace = lsm.getDashSpace();  // по умолчанию 5.0

    // ================= HEADER SECTION =================
    writeCode(0, "SECTION");
    writeCode(2, "HEADER");
    writeCode(9, "$ACADVER");
    writeCode(1, "AC1009");
    writeCode(9, "$HANDLING");
    writeCode(70, 0);
    // Глобальный масштаб типов линий ($LTSCALE)
    writeCode(9, "$LTSCALE");
    writeCode(40, 1.0);
    writeCode(0, "ENDSEC");

    // ================= TABLES SECTION =================
    writeCode(0, "SECTION");
    writeCode(2, "TABLES");

    // Соберем уникальные слои
    QSet<QString> layerNames;
    const auto& primitives = scene.getPrimitives();
    for (const auto& prim : primitives) {
        layerNames.insert(prim->getLayerName());
    }
    if (layerNames.isEmpty()) {
        layerNames.insert("0");
    }

    // LTYPE TABLE — масштабированные паттерны штрихов
    writeCode(0, "TABLE");
    writeCode(2, "LTYPE");
    writeCode(70, 4);

    auto writeLType = [&writeCode](const QString& name, const QString& desc) {
        writeCode(0, "LTYPE");
        writeCode(2, name);
        writeCode(70, 0);
        writeCode(3, desc);
        writeCode(72, 65);  // alignment 'A'
    };

    // CONTINUOUS
    writeLType("CONTINUOUS", "Solid line");
    writeCode(73, 0); writeCode(40, 0.0);

    // DASHED: dash, space
    // Паттерн масштабируется от реальных значений dashLen/dashSpace
    {
        double d = dashLen;       // длина штриха
        double s = dashSpace;     // длина пробела
        double total = d + s;
        writeLType("DASHED", "Dashed __ __ __ __");
        writeCode(73, 2);
        writeCode(40, total);
        writeCode(49, d);
        writeCode(49, -s);
    }

    // DASHDOT: dash, space, dot, space
    {
        double d = dashLen;
        double s = dashSpace / 2.0;  // Уменьшенный пробел для точки
        double total = d + s + 0.0 + s; // dot = 0 длины
        writeLType("DASHDOT", "Dash dot __ . __ . __");
        writeCode(73, 4);
        writeCode(40, total);
        writeCode(49, d);
        writeCode(49, -s);
        writeCode(49, 0.0);   // dot
        writeCode(49, -s);
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
        writeCode(49, -s);
        writeCode(49, 0.0);   // dot 1
        writeCode(49, -s);
        writeCode(49, 0.0);   // dot 2
        writeCode(49, -s);
    }

    writeCode(0, "ENDTAB");

    // LAYER TABLE
    writeCode(0, "TABLE");
    writeCode(2, "LAYER");
    writeCode(70, layerNames.size());
    for (const QString& layerName : layerNames) {
        writeCode(0, "LAYER");
        writeCode(2, layerName);
        writeCode(70, 0);
        writeCode(62, 7);
        writeCode(6, "CONTINUOUS");
    }
    writeCode(0, "ENDTAB");
    writeCode(0, "ENDSEC");

    // ================= BLOCKS SECTION =================
    writeCode(0, "SECTION");
    writeCode(2, "BLOCKS");
    writeCode(0, "ENDSEC");

    // ================= ENTITIES SECTION =================
    writeCode(0, "SECTION");
    writeCode(2, "ENTITIES");

    for (const auto& prim : primitives) {
        
        auto writeCommonProperties = [&writeCode, &prim]() {
            writeCode(8, prim->getLayerName());
            writeCode(6, getDxfLineType(prim->getLineType()));
            writeCode(62, getAutoCadColorIndex(prim->getColor()));
            writeCode(420, getTrueColor24Bit(prim->getColor()));
            // Толщина линии (код 370, в сотых долях мм)
            writeCode(370, getDxfLineWeight(prim->getLineType()));
        };

        auto writeXData = [&writeCode, &prim]() {
            writeCode(999, QString("CLARUSCAD_LTYPE:%1").arg(static_cast<int>(prim->getLineType())));
        };

        // Лямбда для записи POLYLINE из набора точек
        auto writePolyline = [&writeCode, &prim, &writeCommonProperties, &writeXData](
                                  const QVector<QPointF>& pts, bool closed) {
            writeCode(0, "POLYLINE");
            writeCommonProperties();
            writeCode(66, 1);
            writeCode(70, closed ? 1 : 0);
            writeCode(10, 0.0);
            writeCode(20, 0.0);
            writeCode(30, 0.0);
            writeXData();
            for (const auto& pt : pts) {
                writeCode(0, "VERTEX");
                writeCode(8, prim->getLayerName());
                writeCode(10, pt.x());
                writeCode(20, pt.y());
                writeCode(30, 0.0);
            }
            writeCode(0, "SEQEND");
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
                    writeCommonProperties();
                    writeCode(10, start.x());
                    writeCode(20, start.y());
                    writeCode(30, 0.0);
                    writeCode(11, end.x());
                    writeCode(21, end.y());
                    writeCode(31, 0.0);
                    writeXData();
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
                    writeCommonProperties();
                    writeCode(10, circle->getCenter().getX());
                    writeCode(20, circle->getCenter().getY());
                    writeCode(30, 0.0);
                    writeCode(40, circle->getRadius());
                    writeXData();
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
                    writeCommonProperties();
                    writeCode(10, arc->getCenter().getX());
                    writeCode(20, arc->getCenter().getY());
                    writeCode(30, 0.0);
                    writeCode(40, arc->getRadius());
                    writeCode(50, arc->getStartAngle());
                    double endAngle = arc->getStartAngle() + arc->getSpanAngle();
                    writeCode(51, endAngle);
                    writeXData();
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
                    writeCommonProperties();
                    
                    double rx = ellipse->getRadiusX();
                    double ry = ellipse->getRadiusY();
                    double majorRadius = std::max(rx, ry);
                    double minorRadius = std::min(rx, ry);
                    double ratio = minorRadius / majorRadius;
                    
                    double rotationRad = ellipse->getRotation() * 3.14159265358979323846 / 180.0;
                    if (ry > rx) {
                        rotationRad += 3.14159265358979323846 / 2.0;
                    }
                    
                    double dx = majorRadius * std::cos(rotationRad);
                    double dy = majorRadius * std::sin(rotationRad);
                    
                    writeCode(10, ellipse->getCenter().getX());
                    writeCode(20, ellipse->getCenter().getY());
                    writeCode(30, 0.0);
                    writeCode(11, dx);
                    writeCode(21, dy);
                    writeCode(31, 0.0);
                    writeCode(40, ratio);
                    writeCode(41, 0.0);
                    writeCode(42, 2.0 * 3.14159265358979323846);
                    writeXData();
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
                    writeCommonProperties();
                    writeCode(66, 1);
                    writeCode(70, 1);
                    writeCode(10, 0.0);
                    writeCode(20, 0.0);
                    writeCode(30, 0.0);
                    writeXData();
                    
                    const auto& pts = poly->getVertices();
                    for (const auto& pt : pts) {
                        writeCode(0, "VERTEX");
                        writeCode(8, prim->getLayerName());
                        writeCode(10, pt.x());
                        writeCode(20, pt.y());
                        writeCode(30, 0.0);
                    }
                    
                    writeCode(0, "SEQEND");
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
                    writeCommonProperties();
                    writeCode(66, 1);
                    writeCode(70, spline->isClosed() ? 1 : 0);
                    writeCode(10, 0.0);
                    writeCode(20, 0.0);
                    writeCode(30, 0.0);
                    writeXData();
                    
                    auto pts = spline->calculateSplinePoints();
                    for (const auto& pt : pts) {
                        writeCode(0, "VERTEX");
                        writeCode(8, prim->getLayerName());
                        writeCode(10, pt.x());
                        writeCode(20, pt.y());
                        writeCode(30, 0.0);
                    }
                    
                    writeCode(0, "SEQEND");
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
                    writeCommonProperties();
                    writeCode(66, 1);
                    writeCode(70, 1);
                    writeCode(10, 0.0);
                    writeCode(20, 0.0);
                    writeCode(30, 0.0);
                    writeXData();
                    
                    for (int i = 0; i < 4; ++i) {
                        writeCode(0, "VERTEX");
                        writeCode(8, prim->getLayerName());
                        writeCode(10, corners[i].x());
                        writeCode(20, corners[i].y());
                        writeCode(30, 0.0);
                    }
                    
                    writeCode(0, "SEQEND");
                    writeCode(8, prim->getLayerName());
                }
            }
        }
    }

    writeCode(0, "ENDSEC");

    // ================= EOF =================
    writeCode(0, "EOF");

    file.close();
    return true;
}
