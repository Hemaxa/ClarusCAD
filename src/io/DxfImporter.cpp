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
#include <QMap>
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
    return Qt::white;
}

static QColor getQColorFromTrueColor(int trueColor) {
    int r = (trueColor >> 16) & 0xFF;
    int g = (trueColor >> 8) & 0xFF;
    int b = trueColor & 0xFF;
    return QColor(r, g, b);
}

static int getClarusLineType(const QString& dxfType) {
    QString upper = dxfType.toUpper().trimmed();
    if (upper == "SOLID_THIN") return static_cast<int>(LineType::SolidThin);
    if (upper == "SOLID_WAVE") return static_cast<int>(LineType::SolidWave);
    if (upper == "SOLID_KINK") return static_cast<int>(LineType::SolidKink);
    if (upper == "DASHED" || upper.contains("DASH") && !upper.contains("DOT")) return static_cast<int>(LineType::Dashed);
    if (upper == "DASHDOT_THICK") return static_cast<int>(LineType::DashDotThick);
    if (upper == "DASHDOT_THIN" || upper == "DASHDOT" || upper == "CENTER" || upper.contains("CENTER")) return static_cast<int>(LineType::DashDotThin);
    if (upper == "DASHDOTDOT" || upper == "DIVIDE" || upper.contains("PHANTOM") || upper.contains("DIVIDE")) return static_cast<int>(LineType::DashDotDot);
    if (upper == "CONTINUOUS" || upper == "BYLAYER" || upper == "BYBLOCK") return static_cast<int>(LineType::SolidMain);
    // Если тип не распознан — по умолчанию основная
    return static_cast<int>(LineType::SolidMain);
}

// Структура для хранения пары (код, значение) из DXF
struct DxfPair {
    int code;
    QString value;
};

// Структура для временного хранения entity и его данных
struct DxfEntity {
    QString type;
    QVector<DxfPair> data;
};

// Структура для хранения блока (имя + список сущностей)
struct DxfBlock {
    QString name;
    QVector<DxfEntity> entities;
};

// Извлечение общих свойств из данных сущности
struct EntityProps {
    QString layer = "0";
    int lineType = static_cast<int>(LineType::SolidMain);
    QColor color = Qt::white;
};

static EntityProps extractCommonProps(const QVector<DxfPair>& data) {
    EntityProps props;
    bool hasTrueColor = false;
    bool hasClarusData = false;
    int clarusLType = 0;

    for (const auto& d : data) {
        if (d.code == 8) props.layer = d.value;
        else if (d.code == 6) props.lineType = getClarusLineType(d.value);
        else if (d.code == 62 && !hasTrueColor) props.color = getQColorFromAutoCadIndex(d.value.toInt());
        else if (d.code == 420) { props.color = getQColorFromTrueColor(d.value.toInt()); hasTrueColor = true; }
        else if (d.code == 999 && d.value.startsWith("CLARUSCAD_LTYPE:")) {
            hasClarusData = true;
            clarusLType = d.value.mid(16).toInt();
        }
    }

    if (hasClarusData) {
        props.lineType = clarusLType;
    }
    return props;
}

static void applyProps(BasePrimitive* p, const EntityProps& props) {
    p->setLayerName(props.layer);
    p->setColor(props.color);
    p->setLineType(props.lineType);
}

// Прямая обработка одной сущности — добавляет примитивы в сцену
// Возвращает true, если обработана
static void processEntity(const DxfEntity& entity, Scene& scene,
                           const QMap<QString, DxfBlock>& blocks,
                           int depth = 0);

// Обработка полилинии из набора вершин
static void createPolylineSegments(Scene& scene, const QVector<QPointF>& vertices,
                                     bool closed, const EntityProps& props) {
    if (vertices.size() < 2) return;
    for (int i = 0; i < vertices.size() - 1; ++i) {
        auto* seg = new SegmentPrimitive(
            PointPrimitive(vertices[i].x(), vertices[i].y()),
            PointPrimitive(vertices[i+1].x(), vertices[i+1].y()));
        applyProps(seg, props);
        scene.addPrimitive(std::unique_ptr<BasePrimitive>(seg));
    }
    if (closed && vertices.size() > 2) {
        auto* seg = new SegmentPrimitive(
            PointPrimitive(vertices.last().x(), vertices.last().y()),
            PointPrimitive(vertices.first().x(), vertices.first().y()));
        applyProps(seg, props);
        scene.addPrimitive(std::unique_ptr<BasePrimitive>(seg));
    }
}

static void processEntity(const DxfEntity& entity, Scene& scene,
                           const QMap<QString, DxfBlock>& blocks,
                           int depth) {
    // Защита от бесконечной рекурсии (максимум 10 уровней вложенности)
    if (depth > 10) return;

    const auto& data = entity.data;
    EntityProps props = extractCommonProps(data);

    if (entity.type == "LINE") {
        double x1=0, y1=0, x2=0, y2=0;
        for (const auto& d : data) {
            if (d.code == 10) x1 = d.value.toDouble();
            if (d.code == 20) y1 = d.value.toDouble();
            if (d.code == 11) x2 = d.value.toDouble();
            if (d.code == 21) y2 = d.value.toDouble();
        }
        auto* seg = new SegmentPrimitive(PointPrimitive(x1, y1), PointPrimitive(x2, y2));
        applyProps(seg, props);
        scene.addPrimitive(std::unique_ptr<BasePrimitive>(seg));
    }
    else if (entity.type == "CIRCLE") {
        double x=0, y=0, r=0;
        for (const auto& d : data) {
            if (d.code == 10) x = d.value.toDouble();
            if (d.code == 20) y = d.value.toDouble();
            if (d.code == 40) r = d.value.toDouble();
        }
        if (r > 0) {
            auto* circ = new CirclePrimitive(PointPrimitive(x, y), r);
            applyProps(circ, props);
            scene.addPrimitive(std::unique_ptr<BasePrimitive>(circ));
        }
    }
    else if (entity.type == "ARC") {
        double x=0, y=0, r=0, dxfStart=0, dxfEnd=0;
        for (const auto& d : data) {
            if (d.code == 10) x = d.value.toDouble();
            if (d.code == 20) y = d.value.toDouble();
            if (d.code == 40) r = d.value.toDouble();
            if (d.code == 50) dxfStart = d.value.toDouble();
            if (d.code == 51) dxfEnd = d.value.toDouble();
        }
        // Конвертация углов DXF (Y-вверх, CCW) → Qt (Y-вниз, CCW на экране):
        // DXF_angle = -Qt_angle, поэтому обратно: Qt_angle = -DXF_angle.
        // DXF ARC идёт CCW от dxfStart до dxfEnd.
        // Экспортёр делал: dxfStart = -(qtStart+qtSpan), dxfEnd = -qtStart.
        // Обратно: qtStart = -dxfEnd, qtSpan = dxfEnd - dxfStart.
        double qtStart = -dxfEnd;
        double qtSpan = dxfEnd - dxfStart;
        if (qtSpan < 0) qtSpan += 360.0;
        // Нормализуем qtStart в [0, 360)
        qtStart = std::fmod(qtStart, 360.0);
        if (qtStart < 0) qtStart += 360.0;
        if (r > 0) {
            auto* arc = new ArcPrimitive(PointPrimitive(x, y), r, qtStart, qtSpan);
            applyProps(arc, props);
            scene.addPrimitive(std::unique_ptr<BasePrimitive>(arc));
        }
    }
    else if (entity.type == "ELLIPSE") {
        double cx=0, cy=0, dx=0, dy=0, ratio=1.0;
        for (const auto& d : data) {
            if (d.code == 10) cx = d.value.toDouble();
            if (d.code == 20) cy = d.value.toDouble();
            if (d.code == 11) dx = d.value.toDouble();
            if (d.code == 21) dy = d.value.toDouble();
            if (d.code == 40) ratio = d.value.toDouble();
        }
        double majorRadius = std::sqrt(dx*dx + dy*dy);
        double minorRadius = majorRadius * ratio;
        // Конвертация: DXF rotation (Y-вверх) → Qt rotation (Y-вниз) = negation
        double rotation = -(std::atan2(dy, dx) * 180.0 / M_PI);
        if (majorRadius > 0) {
            auto* ell = new EllipsePrimitive(PointPrimitive(cx, cy), majorRadius, minorRadius, rotation);
            applyProps(ell, props);
            scene.addPrimitive(std::unique_ptr<BasePrimitive>(ell));
        }
    }
    else if (entity.type == "SPLINE") {
        QVector<QPointF> splPoints;
        double curX = 0; bool curXSet = false;
        bool isClosed = false;
        for (const auto& d : data) {
            if (d.code == 70) isClosed = (d.value.toInt() & 1);
            if (d.code == 10) { curX = d.value.toDouble(); curXSet = true; }
            if (d.code == 20 && curXSet) { splPoints.append(QPointF(curX, d.value.toDouble())); curXSet = false; }
        }
        if (splPoints.size() >= 2) {
            auto* spline = new SplinePrimitive(splPoints);
            spline->setClosed(isClosed);
            applyProps(spline, props);
            scene.addPrimitive(std::unique_ptr<BasePrimitive>(spline));
        }
    }
    else if (entity.type == "LWPOLYLINE") {
        QVector<QPointF> pts;
        bool closed = false;
        double curX = 0; bool hasX = false;
        for (const auto& d : data) {
            if (d.code == 70) closed = (d.value.toInt() & 1);
            else if (d.code == 10) {
                if (hasX) pts.append(QPointF(curX, 0.0));
                curX = d.value.toDouble();
                hasX = true;
            }
            else if (d.code == 20 && hasX) {
                pts.append(QPointF(curX, d.value.toDouble()));
                hasX = false;
            }
        }
        createPolylineSegments(scene, pts, closed, props);
    }
    else if (entity.type == "INSERT") {
        // INSERT — вставка блока. Находим блок по имени и обрабатываем его сущности
        QString blockName;
        double insertX = 0, insertY = 0;
        double scaleX = 1.0, scaleY = 1.0;
        double rotation = 0.0;
        for (const auto& d : data) {
            if (d.code == 2) blockName = d.value;
            else if (d.code == 10) insertX = d.value.toDouble();
            else if (d.code == 20) insertY = d.value.toDouble();
            else if (d.code == 41) scaleX = d.value.toDouble();
            else if (d.code == 42) scaleY = d.value.toDouble();
            else if (d.code == 50) rotation = d.value.toDouble();
        }

        if (blocks.contains(blockName)) {
            const DxfBlock& block = blocks[blockName];
            for (const auto& blockEntity : block.entities) {
                processEntity(blockEntity, scene, blocks, depth + 1);
            }
        }
    }
    // Неизвестные типы (POINT, VIEWPORT и др.) – пропускаем
}

bool DxfImporter::importDxfToScene(Scene& scene, const QString& filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return false;
    }

    // 1. Читаем весь файл в массив пар (код, значение)
    QTextStream in(&file);
    QVector<DxfPair> pairs;
    pairs.reserve(20000);

    while (!in.atEnd()) {
        QString codeStr = in.readLine().trimmed();
        if (codeStr.isEmpty() && in.atEnd()) break;
        if (in.atEnd()) break;

        QString valueStr = in.readLine().trimmed();

        bool ok = false;
        int code = codeStr.toInt(&ok);
        if (!ok) continue;

        pairs.append({code, valueStr});
    }
    file.close();

    int totalPairs = pairs.size();
    if (totalPairs == 0) return false;

    // 2. Вспомогательная функция: собрать данные одной entity начиная с позиции после entity type
    //    Возвращает позицию следующего кода 0 (entity type) или конца секции
    auto collectEntityData = [&](int start) -> QPair<QVector<DxfPair>, int> {
        QVector<DxfPair> entityData;
        int pos = start;
        while (pos < totalPairs && pairs[pos].code != 0) {
            entityData.append(pairs[pos]);
            ++pos;
        }
        return {entityData, pos};
    };

    // 3. Парсинг BLOCKS секции — собираем блоки и их содержимое
    QMap<QString, DxfBlock> blocks;

    // Ищем начало секции BLOCKS
    int i = 0;
    while (i < totalPairs - 1) {
        if (pairs[i].code == 0 && pairs[i].value == "SECTION" &&
            pairs[i+1].code == 2 && pairs[i+1].value == "BLOCKS") {
            i += 2;
            break;
        }
        ++i;
    }

    // Парсим блоки
    while (i < totalPairs) {
        if (pairs[i].code == 0 && pairs[i].value == "ENDSEC") {
            ++i;
            break;
        }

        if (pairs[i].code == 0 && pairs[i].value == "BLOCK") {
            ++i;
            // Собираем данные BLOCK-записи (до следующего кода 0)
            auto [blockData, nextPos] = collectEntityData(i);
            i = nextPos;

            // Извлекаем имя блока (код 2 или код 3)
            QString blockName;
            for (const auto& d : blockData) {
                if (d.code == 2) { blockName = d.value; break; }
            }
            // Если имя не найдено в коде 2, ищем в коде 3
            if (blockName.isEmpty()) {
                for (const auto& d : blockData) {
                    if (d.code == 3) { blockName = d.value; break; }
                }
            }

            // Пропускаем системные блоки (Model_Space, Paper_Space)
            bool isSystemBlock = blockName.startsWith("*Model_Space") ||
                                  blockName.startsWith("*Paper_Space");

            DxfBlock currentBlock;
            currentBlock.name = blockName;

            // Собираем сущности внутри блока до ENDBLK
            // Также собираем POLYLINE + VERTEX + SEQEND последовательности
            bool readingPolyline = false;
            QVector<QPointF> polyVertices;
            QVector<DxfPair> polyHeaderData;

            while (i < totalPairs) {
                if (pairs[i].code == 0) {
                    QString entityType = pairs[i].value;

                    if (entityType == "ENDBLK") {
                        // Финализируем незакрытую полилинию
                        if (readingPolyline && !polyVertices.isEmpty()) {
                            DxfEntity polyEntity;
                            polyEntity.type = "LWPOLYLINE";
                            // Конвертируем POLYLINE+VERTEX в формат LWPOLYLINE для обработки
                            polyEntity.data = polyHeaderData;
                            for (const auto& pt : polyVertices) {
                                polyEntity.data.append({10, QString::number(pt.x(), 'f', 10)});
                                polyEntity.data.append({20, QString::number(pt.y(), 'f', 10)});
                            }
                            currentBlock.entities.append(polyEntity);
                            readingPolyline = false;
                            polyVertices.clear();
                        }

                        ++i;
                        // Пропускаем данные ENDBLK
                        while (i < totalPairs && pairs[i].code != 0) ++i;
                        break;
                    }

                    ++i; // Переходим к данным
                    auto [entData, nextPos2] = collectEntityData(i);
                    i = nextPos2;

                    if (entityType == "POLYLINE") {
                        readingPolyline = true;
                        polyVertices.clear();
                        polyHeaderData = entData;
                    }
                    else if (entityType == "VERTEX" && readingPolyline) {
                        double px = 0, py = 0;
                        for (const auto& d : entData) {
                            if (d.code == 10) px = d.value.toDouble();
                            if (d.code == 20) py = d.value.toDouble();
                        }
                        polyVertices.append(QPointF(px, py));
                    }
                    else if (entityType == "SEQEND" && readingPolyline) {
                        // Финализируем POLYLINE → конвертируем в формат LWPOLYLINE
                        DxfEntity polyEntity;
                        polyEntity.type = "LWPOLYLINE";
                        polyEntity.data = polyHeaderData;
                        for (const auto& pt : polyVertices) {
                            polyEntity.data.append({10, QString::number(pt.x(), 'f', 10)});
                            polyEntity.data.append({20, QString::number(pt.y(), 'f', 10)});
                        }
                        currentBlock.entities.append(polyEntity);
                        readingPolyline = false;
                        polyVertices.clear();
                    }
                    else {
                        // Обычная сущность (LINE, CIRCLE, ARC, ELLIPSE, SPLINE, INSERT, ...)
                        if (!readingPolyline) {
                            DxfEntity ent;
                            ent.type = entityType;
                            ent.data = entData;
                            currentBlock.entities.append(ent);
                        }
                    }
                } else {
                    ++i;
                }
            }

            if (!isSystemBlock && !blockName.isEmpty()) {
                blocks[blockName] = currentBlock;
            }
        } else {
            ++i;
        }
    }

    // 4. Парсинг ENTITIES секции
    // Ищем начало секции ENTITIES
    while (i < totalPairs - 1) {
        if (pairs[i].code == 0 && pairs[i].value == "SECTION" &&
            pairs[i+1].code == 2 && pairs[i+1].value == "ENTITIES") {
            i += 2;
            break;
        }
        ++i;
    }

    // Переменные для POLYLINE в ENTITIES
    bool readingPolyline = false;
    QVector<QPointF> currentPolyVertices;
    QVector<DxfPair> currentPolyHeaderData;

    while (i < totalPairs) {
        if (pairs[i].code == 0 && (pairs[i].value == "ENDSEC" || pairs[i].value == "EOF")) {
            // Финализируем незакрытую полилинию
            if (readingPolyline && !currentPolyVertices.isEmpty()) {
                DxfEntity polyEntity;
                polyEntity.type = "LWPOLYLINE";
                polyEntity.data = currentPolyHeaderData;
                for (const auto& pt : currentPolyVertices) {
                    polyEntity.data.append({10, QString::number(pt.x(), 'f', 10)});
                    polyEntity.data.append({20, QString::number(pt.y(), 'f', 10)});
                }
                processEntity(polyEntity, scene, blocks);
                readingPolyline = false;
            }
            break;
        }

        if (pairs[i].code != 0) {
            ++i;
            continue;
        }

        QString entityType = pairs[i].value;
        ++i;

        auto [entData, nextPos] = collectEntityData(i);
        i = nextPos;

        if (entityType == "POLYLINE") {
            readingPolyline = true;
            currentPolyVertices.clear();
            currentPolyHeaderData = entData;
        }
        else if (entityType == "VERTEX" && readingPolyline) {
            double px = 0, py = 0;
            for (const auto& d : entData) {
                if (d.code == 10) px = d.value.toDouble();
                if (d.code == 20) py = d.value.toDouble();
            }
            currentPolyVertices.append(QPointF(px, py));
        }
        else if (entityType == "SEQEND" && readingPolyline) {
            DxfEntity polyEntity;
            polyEntity.type = "LWPOLYLINE";
            polyEntity.data = currentPolyHeaderData;
            for (const auto& pt : currentPolyVertices) {
                polyEntity.data.append({10, QString::number(pt.x(), 'f', 10)});
                polyEntity.data.append({20, QString::number(pt.y(), 'f', 10)});
            }
            processEntity(polyEntity, scene, blocks);
            readingPolyline = false;
            currentPolyVertices.clear();
        }
        else if (!readingPolyline) {
            DxfEntity ent;
            ent.type = entityType;
            ent.data = entData;
            processEntity(ent, scene, blocks);
        }
    }

    return true;
}
