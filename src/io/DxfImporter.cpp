//DxfImporter.cpp - реализация импорта сцены из формата DXF

#include "DxfImporter.h"
#include "Scene.h"
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
#include <QColor>
#include <vector>
#include <cmath>
#include <algorithm>
#include <QDebug>

static QColor getQColorFromAutoCadIndex(int aci) {
    if (aci == 1) return Qt::red;
    if (aci == 2) return Qt::yellow;
    if (aci == 3) return Qt::green;
    if (aci == 4) return Qt::cyan;
    if (aci == 5) return Qt::blue;
    if (aci == 6) return Qt::magenta;
    if (aci == 7) return Qt::white;
    if (aci == 0) return Qt::black; 
    return Qt::white; // 256 (BYLAYER) and others defaults to white
}

static QColor getQColorFromTrueColor(int trueColor) {
    int r = (trueColor >> 16) & 0xFF;
    int g = (trueColor >> 8) & 0xFF;
    int b = trueColor & 0xFF;
    return QColor(r, g, b);
}

static int getClarusLineType(const QString& dxfType) {
    if (dxfType == "SOLID_THIN") return static_cast<int>(LineType::SolidThin);
    if (dxfType == "SOLID_WAVE") return static_cast<int>(LineType::SolidWave);
    if (dxfType == "SOLID_KINK") return static_cast<int>(LineType::SolidKink);
    if (dxfType == "DASHED") return static_cast<int>(LineType::Dashed);
    if (dxfType == "DASHDOT_THICK") return static_cast<int>(LineType::DashDotThick);
    if (dxfType == "DASHDOT_THIN" || dxfType == "DASHDOT") return static_cast<int>(LineType::DashDotThin);
    if (dxfType == "DASHDOTDOT" || dxfType == "DIVIDE") return static_cast<int>(LineType::DashDotDot);
    return static_cast<int>(LineType::SolidMain); // Default fallback covers CONTINUOUS
}

bool DxfImporter::importDxfToScene(Scene& scene, const QString& filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return false;
    }

    QTextStream in(&file);
    bool inEntitiesSection = false;
    
    // Вспомогательная лямбда для чтения следующего кода/значения
    auto readPair = [&in](int& code, QString& value) -> bool {
        QString codeStr = in.readLine().trimmed();
        if (codeStr.isNull()) return false;
        code = codeStr.toInt();
        value = in.readLine().trimmed();
        return true;
    };

    int code;
    QString value;

    // Временные структуры для полилинии
    bool readingPolyline = false;
    QVector<QPointF> currentPolyVertices;
    QString currentPolyLayer = "0";
    int currentPolyLineType = static_cast<int>(LineType::SolidMain);
    QColor currentPolyColor = Qt::white;
    bool currentPolyClosed = false;

    auto applyProps = [](BasePrimitive* p, const QString& lay, const QColor& c, int ltype) {
        p->setLayerName(lay);
        p->setColor(c);
        p->setLineType(ltype);
    };

    while (!in.atEnd()) {
        if (!readPair(code, value)) break;

        if (code == 0 && value == "SECTION") {
            if (!readPair(code, value)) break;
            if (code == 2 && value == "ENTITIES") {
                inEntitiesSection = true;
                continue;
            }
        }

        if (code == 0 && value == "ENDSEC" && inEntitiesSection) {
            inEntitiesSection = false;
        }

        if (inEntitiesSection && code == 0) {
            QString entityType = value;
            
            QVector<QPair<int, QString>> entityData;
            
            while (!in.atEnd()) {
                qint64 pos = in.pos();
                QString nextCodeStr = in.readLine().trimmed();
                if (nextCodeStr.isNull()) break;
                
                int nextCode = nextCodeStr.toInt();
                if (nextCode == 0) {
                    in.seek(pos);
                    break;
                }
                
                QString nextValue = in.readLine().trimmed();
                entityData.append(qMakePair(nextCode, nextValue));
            }

            QString layer = "0";
            int ltype = static_cast<int>(LineType::SolidMain);
            QColor color = Qt::white;
            bool hasTrueColor = false;
            
            bool hasClarusData = false;
            int clarusLType = 0;

            for (const auto& d : entityData) {
                if (d.first == 8) layer = d.second;
                else if (d.first == 6) ltype = getClarusLineType(d.second);
                else if (d.first == 62 && !hasTrueColor) color = getQColorFromAutoCadIndex(d.second.toInt());
                else if (d.first == 420) { color = getQColorFromTrueColor(d.second.toInt()); hasTrueColor = true; }
                else if (d.first == 999 && d.second.startsWith("CLARUSCAD_LTYPE:")) {
                    hasClarusData = true;
                    clarusLType = d.second.mid(16).toInt();
                }
            }
            
            if (hasClarusData) {
                ltype = clarusLType;
            }

            if (entityType == "POINT") {
                // PointPrimitive is not a BasePrimitive in ClarusCAD, ignoring.
            }
            else if (entityType == "LINE") {
                double x1=0, y1=0, x2=0, y2=0;
                for (const auto& d : entityData) {
                    if (d.first == 10) x1 = d.second.toDouble();
                    if (d.first == 20) y1 = d.second.toDouble();
                    if (d.first == 11) x2 = d.second.toDouble();
                    if (d.first == 21) y2 = d.second.toDouble();
                }
                auto* seg = new SegmentPrimitive(PointPrimitive(x1, y1), PointPrimitive(x2, y2));
                applyProps(seg, layer, color, ltype);
                scene.addPrimitive(std::unique_ptr<BasePrimitive>(seg));
            }
            else if (entityType == "CIRCLE") {
                double x=0, y=0, r=0;
                for (const auto& d : entityData) {
                    if (d.first == 10) x = d.second.toDouble();
                    if (d.first == 20) y = d.second.toDouble();
                    if (d.first == 40) r = d.second.toDouble();
                }
                auto* circ = new CirclePrimitive(PointPrimitive(x, y), r);
                applyProps(circ, layer, color, ltype);
                scene.addPrimitive(std::unique_ptr<BasePrimitive>(circ));
            }
            else if (entityType == "ARC") {
                double x=0, y=0, r=0, start=0, endAngle=0;
                for (const auto& d : entityData) {
                    if (d.first == 10) x = d.second.toDouble();
                    if (d.first == 20) y = d.second.toDouble();
                    if (d.first == 40) r = d.second.toDouble();
                    if (d.first == 50) start = d.second.toDouble();
                    if (d.first == 51) endAngle = d.second.toDouble();
                }
                double span = endAngle - start;
                if (span < 0) span += 360.0;
                auto* arc = new ArcPrimitive(PointPrimitive(x, y), r, start, span);
                applyProps(arc, layer, color, ltype);
                scene.addPrimitive(std::unique_ptr<BasePrimitive>(arc));
            }
            else if (entityType == "ELLIPSE") {
                double cx=0, cy=0, dx=0, dy=0, ratio=1.0;
                for (const auto& d : entityData) {
                    if (d.first == 10) cx = d.second.toDouble();
                    if (d.first == 20) cy = d.second.toDouble();
                    if (d.first == 11) dx = d.second.toDouble();
                    if (d.first == 21) dy = d.second.toDouble();
                    if (d.first == 40) ratio = d.second.toDouble();
                }
                double majorRadius = std::sqrt(dx*dx + dy*dy);
                double minorRadius = majorRadius * ratio;
                double rotation = std::atan2(dy, dx) * 180.0 / 3.14159265358979323846;
                auto* ell = new EllipsePrimitive(PointPrimitive(cx, cy), majorRadius, minorRadius, rotation);
                applyProps(ell, layer, color, ltype);
                scene.addPrimitive(std::unique_ptr<BasePrimitive>(ell));
            }
            else if (entityType == "SPLINE") {
                QVector<QPointF> splPoints;
                double curX = 0; bool curXSet = false;
                bool isClosed = false;
                for (const auto& d : entityData) {
                    if (d.first == 70) isClosed = (d.second.toInt() & 1);
                    if (d.first == 10) { curX = d.second.toDouble(); curXSet = true; }
                    if (d.first == 20 && curXSet) { splPoints.append(QPointF(curX, d.second.toDouble())); curXSet = false; }
                }
                if (splPoints.size() >= 2) {
                    auto* spline = new SplinePrimitive(splPoints);
                    spline->setClosed(isClosed);
                    applyProps(spline, layer, color, ltype);
                    scene.addPrimitive(std::unique_ptr<BasePrimitive>(spline));
                }
            }
            else if (entityType == "POLYLINE") {
                readingPolyline = true;
                currentPolyLayer = layer;
                currentPolyColor = color;
                currentPolyLineType = ltype;
                currentPolyVertices.clear();
                currentPolyClosed = false;
                for (const auto& d : entityData) {
                    if (d.first == 70) currentPolyClosed = (d.second.toInt() & 1);
                }
            }
            else if (entityType == "VERTEX" && readingPolyline) {
                double px=0, py=0;
                for (const auto& d : entityData) {
                    if (d.first == 10) px = d.second.toDouble();
                    if (d.first == 20) py = d.second.toDouble();
                }
                currentPolyVertices.append(QPointF(px, py));
            }
            else if (entityType == "SEQEND" && readingPolyline) {
                readingPolyline = false;
                if (currentPolyVertices.size() >= 2) {
                    for (int i = 0; i < currentPolyVertices.size() - 1; ++i) {
                        auto* seg = new SegmentPrimitive(PointPrimitive(currentPolyVertices[i].x(), currentPolyVertices[i].y()), 
                                                         PointPrimitive(currentPolyVertices[i+1].x(), currentPolyVertices[i+1].y()));
                        applyProps(seg, currentPolyLayer, currentPolyColor, currentPolyLineType);
                        scene.addPrimitive(std::unique_ptr<BasePrimitive>(seg));
                    }
                    if (currentPolyClosed && currentPolyVertices.size() > 2) {
                        auto* seg = new SegmentPrimitive(PointPrimitive(currentPolyVertices.last().x(), currentPolyVertices.last().y()), 
                                                         PointPrimitive(currentPolyVertices.first().x(), currentPolyVertices.first().y()));
                        applyProps(seg, currentPolyLayer, currentPolyColor, currentPolyLineType);
                        scene.addPrimitive(std::unique_ptr<BasePrimitive>(seg));
                    }
                }
            }
        }
    }

    file.close();
    return true;
}
