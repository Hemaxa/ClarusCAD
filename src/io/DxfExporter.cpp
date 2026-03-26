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
    case LineType::SolidThin:     return "CONTINUOUS"; // To make it visible as solid in T-Flex, actual type preserved in XDATA
    case LineType::SolidWave:     return "CONTINUOUS"; // Preserved in XDATA
    case LineType::SolidKink:     return "CONTINUOUS"; // Preserved in XDATA
    case LineType::Dashed:        return "DASHED";
    case LineType::DashDotThick:  return "DASHDOT";
    case LineType::DashDotThin:   return "DASHDOT";
    case LineType::DashDotDot:    return "DIVIDE"; // DIVIDE is standard dash-dot-dot
    default:                      return "CONTINUOUS";
    }
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

    // ================= HEADER SECTION =================
    writeCode(0, "SECTION");
    writeCode(2, "HEADER");
    // Версия DXF (AutoCAD 2013 - AC1027) для поддержки TrueColor и улучшенных типов
    writeCode(9, "$ACADVER");
    writeCode(1, "AC1027");
    writeCode(0, "ENDSEC");

    // ================= CLASSES SECTION =================
    // Пропустим, объекты простые

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

    // LTYPE TABLE
    writeCode(0, "TABLE");
    writeCode(2, "LTYPE");
    writeCode(70, 8); // Number of linetypes: CONTINUOUS + 7 custom

    auto writeLType = [&writeCode](const QString& name, const QString& desc) {
        writeCode(0, "LTYPE");
        writeCode(2, name);
        writeCode(70, 0);       // standard flag
        writeCode(3, desc);     // description
        writeCode(72, 65);      // alignment code 'A' (65)
    };

    writeLType("CONTINUOUS", "Solid main line");
    writeCode(73, 0); writeCode(40, 0.0);

    writeLType("SOLID_THIN", "Solid thin line");
    writeCode(73, 0); writeCode(40, 0.0);

    writeLType("SOLID_WAVE", "Solid wavy line");
    writeCode(73, 0); writeCode(40, 0.0);

    writeLType("SOLID_KINK", "Solid zigzag line");
    writeCode(73, 0); writeCode(40, 0.0);

    writeLType("DASHED", "Dashed __ __ __ __");
    writeCode(73, 2); 
    writeCode(40, 0.75); 
    writeCode(49, 0.5); 
    writeCode(49, -0.25); 

    writeLType("DASHDOT_THICK", "Dash dot thick __ . __ . __");
    writeCode(73, 4); 
    writeCode(40, 1.0); 
    writeCode(49, 0.5); 
    writeCode(49, -0.25); 
    writeCode(49, 0.0); 
    writeCode(49, -0.25); 

    writeLType("DASHDOT_THIN", "Dash dot thin __ . __ . __");
    writeCode(73, 4); 
    writeCode(40, 1.0); 
    writeCode(49, 0.5); 
    writeCode(49, -0.25); 
    writeCode(49, 0.0); 
    writeCode(49, -0.25); 
    
    writeLType("DIVIDE", "Dash dot dot __ . . __ . . __");
    writeCode(73, 6); 
    writeCode(40, 1.25); 
    writeCode(49, 0.5); 
    writeCode(49, -0.25); 
    writeCode(49, 0.0); 
    writeCode(49, -0.25); 
    writeCode(49, 0.0); 
    writeCode(49, -0.25); 

    writeCode(0, "ENDTAB");
    
    // APPID TABLE
    writeCode(0, "TABLE");
    writeCode(2, "APPID");
    writeCode(70, 1);
    
    writeCode(0, "APPID");
    writeCode(2, "CLARUSCAD");
    writeCode(70, 0);
    
    writeCode(0, "ENDTAB");

    // LAYER TABLE
    writeCode(0, "TABLE");
    writeCode(2, "LAYER");
    writeCode(70, layerNames.size());
    for (const QString& layerName : layerNames) {
        writeCode(0, "LAYER");
        writeCode(2, layerName);
        writeCode(70, 0); // Флаги состояния (0 = нормальное)
        writeCode(62, 7); // Цвет слоя (по умолчанию белый)
        writeCode(6, "CONTINUOUS"); // Тип линии слоя
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
            writeCode(8, prim->getLayerName()); // Имя слоя (код 8)
            writeCode(6, getDxfLineType(prim->getLineType())); // Тип линии (код 6)
            writeCode(62, getAutoCadColorIndex(prim->getColor())); // Код цвета ACI (код 62)
            writeCode(420, getTrueColor24Bit(prim->getColor())); // TrueColor RGB (код 420)
        };

        auto writeXData = [&writeCode, &prim]() {
            writeCode(1001, "CLARUSCAD");
            writeCode(1071, static_cast<int>(prim->getLineType()));
        };

        PrimitiveType type = prim->getType();

        if (type == PrimitiveType::Segment) {
            auto* segment = dynamic_cast<SegmentPrimitive*>(prim.get());
            if (segment) {
                writeCode(0, "LINE");
                writeCommonProperties();
                writeCode(10, segment->getStart().getX());
                writeCode(20, segment->getStart().getY());
                writeCode(30, 0.0);
                writeCode(11, segment->getEnd().getX());
                writeCode(21, segment->getEnd().getY());
                writeCode(31, 0.0);
                writeXData();
            }
        }
        else if (type == PrimitiveType::Circle) {
            auto* circle = dynamic_cast<CirclePrimitive*>(prim.get());
            if (circle) {
                writeCode(0, "CIRCLE");
                writeCommonProperties();
                writeCode(10, circle->getCenter().getX());
                writeCode(20, circle->getCenter().getY());
                writeCode(30, 0.0);
                writeCode(40, circle->getRadius());
                writeXData();
            }
        }
        else if (type == PrimitiveType::Arc) {
            auto* arc = dynamic_cast<ArcPrimitive*>(prim.get());
            if (arc) {
                writeCode(0, "ARC");
                writeCommonProperties();
                writeCode(10, arc->getCenter().getX());
                writeCode(20, arc->getCenter().getY());
                writeCode(30, 0.0);
                writeCode(40, arc->getRadius());
                writeCode(50, arc->getStartAngle()); // DXF ожидает градусы
                // DXF Arc start angle is startAngle
                // DXF Arc end angle is startAngle + spanAngle
                double endAngle = arc->getStartAngle() + arc->getSpanAngle();
                writeCode(51, endAngle);
                writeXData();
            }
        }
        else if (type == PrimitiveType::Ellipse) {
            auto* ellipse = dynamic_cast<EllipsePrimitive*>(prim.get());
            if (ellipse) {
                writeCode(0, "ELLIPSE");
                writeCommonProperties();
                
                // Для DXF: Эллипс задается центром(10,20,30), смещением конца ГЛАВНОЙ оси(11,21,31) 
                // и отношением малой оси к главной (40).
                
                double rx = ellipse->getRadiusX();
                double ry = ellipse->getRadiusY();
                double majorRadius = std::max(rx, ry);
                double minorRadius = std::min(rx, ry);
                double ratio = minorRadius / majorRadius;
                
                // Угол поворота главной оси (в ClarusCAD это вращение в градусах)
                double rotationRad = ellipse->getRotation() * 3.14159265358979323846 / 180.0;
                
                // Если ry > rx, значит главная ось по Y, мы должны добавить 90 градусов
                if (ry > rx) {
                    rotationRad += 3.14159265358979323846 / 2.0;
                }
                
                // Вычисляем смещение конца главной оси от центра
                double dx = majorRadius * std::cos(rotationRad);
                double dy = majorRadius * std::sin(rotationRad);
                
                writeCode(10, ellipse->getCenter().getX());
                writeCode(20, ellipse->getCenter().getY());
                writeCode(30, 0.0);
                
                writeCode(11, dx);
                writeCode(21, dy);
                writeCode(31, 0.0);
                
                writeCode(40, ratio);
                
                // Начальный и конечный параметры (от 0 до 2*pi)
                writeCode(41, 0.0);
                writeCode(42, 2.0 * 3.14159265358979323846);
                writeXData();
            }
        }
        else if (type == PrimitiveType::Polygon) {
            auto* poly = dynamic_cast<PolygonPrimitive*>(prim.get());
            if (poly) {
                // В DXF AC1009 многоугольник экспортируется как POLYLINE
                writeCode(0, "POLYLINE");
                writeCommonProperties();
                writeCode(66, 1);               // Флаг что за полилинией следуют VERTEX
                writeCode(70, 1);               // Флаг 1 = замкнутая полилиния
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
        else if (type == PrimitiveType::Spline) {
            auto* spline = dynamic_cast<SplinePrimitive*>(prim.get());
            if (spline) {
                // В DXF AC1009 сплайн (SPLINE) не поддерживается. Экспортируем как полилинию по точкам,
                // чтобы в точности сохранить форму кривой Catmull-Rom.
                writeCode(0, "POLYLINE");
                writeCommonProperties();
                writeCode(66, 1);               // Флаг что за полилинией следуют VERTEX
                writeCode(70, spline->isClosed() ? 1 : 0); // Флаг 1 = замкнутая полилиния
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
        else if (type == PrimitiveType::Rectangle) {
            auto* rect = dynamic_cast<RectanglePrimitive*>(prim.get());
            if (rect) {
                // Экспортируем прямоугольник как замкнутую POLYLINE
                writeCode(0, "POLYLINE");
                writeCommonProperties();
                writeCode(66, 1);               // Флаг что за полилинией следуют VERTEX
                writeCode(70, 1);               // Флаг 1 = замкнутая полилиния
                writeCode(10, 0.0);
                writeCode(20, 0.0);
                writeCode(30, 0.0);
                
                // Центр, ширина, высота и угол (надо вычислить 4 угла)
                double w = rect->getWidth();
                double h = rect->getHeight();
                double rot = rect->getRotation() * M_PI / 180.0;
                double cx = rect->getCenter().getX();
                double cy = rect->getCenter().getY();
                
                // 4 вершины без поворота (отсчет: левый-верхний, правый-верхний, правый-нижний, левый-нижний)
                QVector<QPointF> pts(4);
                pts[0] = QPointF(-w/2, -h/2);
                pts[1] = QPointF(w/2, -h/2);
                pts[2] = QPointF(w/2, h/2);
                pts[3] = QPointF(-w/2, h/2);
                
                writeXData();
                
                for (int i = 0; i < 4; ++i) {
                    double x = pts[i].x() * std::cos(rot) - pts[i].y() * std::sin(rot) + cx;
                    double y = pts[i].x() * std::sin(rot) + pts[i].y() * std::cos(rot) + cy;
                    
                    writeCode(0, "VERTEX");
                    writeCode(8, prim->getLayerName());
                    writeCode(10, x);
                    writeCode(20, y);
                    writeCode(30, 0.0);
                }
                
                writeCode(0, "SEQEND");
                writeCode(8, prim->getLayerName());
            }
        }
    }

    writeCode(0, "ENDSEC");

    // ================= EOF =================
    writeCode(0, "EOF");

    file.close();
    return true;
}
