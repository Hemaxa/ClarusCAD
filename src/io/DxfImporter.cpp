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
#include "../model/primitives/dimensions/BaseDimensionPrimitive.h"
#include "../model/primitives/dimensions/LinearDimensionPrimitive.h"
#include "../model/primitives/dimensions/RadialDimensionPrimitive.h"
#include "../model/primitives/dimensions/AngularDimensionPrimitive.h"
#include "EnumManager.h"
#include "../view/managers/SettingsManager.h"

#include <QFile>
#include <QTextStream>
#include <QColor>
#include <QMap>
#include <QHash>
#include <vector>
#include <cmath>
#include <algorithm>
#include <QDebug>
#include <QFileInfo>

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

struct ImportedEntityRef {
    BasePrimitive* primary = nullptr;
    QVector<BasePrimitive*> edges;
};

struct DimensionAssociationMap {
    QHash<int, ImportedEntityRef> byId;
};

struct DxfMeta {
    QHash<QString, QString> values;
};

struct ParsedDimensionText {
    QString customText;
    QString prefix;
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

static DxfMeta extractMeta(const QVector<DxfPair>& data)
{
    DxfMeta meta;
    for (const auto& d : data) {
        if (d.code != 999) continue;
        const int sep = d.value.indexOf(':');
        if (sep <= 0) continue;
        meta.values.insert(d.value.left(sep), d.value.mid(sep + 1));
    }
    return meta;
}

static QString dataValue(const QVector<DxfPair>& data, int code, const QString& fallback = QString())
{
    for (const auto& d : data) {
        if (d.code == code) return d.value;
    }
    return fallback;
}

static bool dataHasCode(const QVector<DxfPair>& data, int code)
{
    for (const auto& d : data) {
        if (d.code == code) return true;
    }
    return false;
}

static double dataDouble(const QVector<DxfPair>& data, int code, double fallback = 0.0)
{
    bool ok = false;
    const double value = dataValue(data, code).toDouble(&ok);
    return ok ? value : fallback;
}

static int dataInt(const QVector<DxfPair>& data, int code, int fallback = 0)
{
    bool ok = false;
    const int value = dataValue(data, code).toInt(&ok);
    return ok ? value : fallback;
}

static QPointF dataPoint(const QVector<DxfPair>& data, int xCode, int yCode, const QPointF& fallback = QPointF())
{
    QPointF result = fallback;
    bool hasX = false;
    bool hasY = false;
    for (const auto& d : data) {
        if (d.code == xCode) { result.setX(d.value.toDouble()); hasX = true; }
        else if (d.code == yCode) { result.setY(d.value.toDouble()); hasY = true; }
    }
    return (hasX || hasY) ? result : fallback;
}

static bool hasSubclassMarker(const QVector<DxfPair>& data, const QString& subclass)
{
    for (const auto& d : data) {
        if (d.code == 100 && d.value.trimmed() == subclass) return true;
    }
    return false;
}

static double normalizeAngleDegrees(double angle)
{
    angle = std::fmod(angle, 360.0);
    if (angle < 0.0) angle += 360.0;
    return angle;
}

static bool lineIntersectionPoint(const QPointF& a1, const QPointF& a2,
                                  const QPointF& b1, const QPointF& b2,
                                  QPointF& outPoint)
{
    const double x1 = a1.x(), y1 = a1.y();
    const double x2 = a2.x(), y2 = a2.y();
    const double x3 = b1.x(), y3 = b1.y();
    const double x4 = b2.x(), y4 = b2.y();
    const double denom = (x1 - x2) * (y3 - y4) - (y1 - y2) * (x3 - x4);
    if (std::abs(denom) < 1e-9) return false;

    const double det1 = x1 * y2 - y1 * x2;
    const double det2 = x3 * y4 - y3 * x4;
    outPoint.setX((det1 * (x3 - x4) - (x1 - x2) * det2) / denom);
    outPoint.setY((det1 * (y3 - y4) - (y1 - y2) * det2) / denom);
    return true;
}

static QPointF fartherRayPoint(const QPointF& center, const QPointF& a, const QPointF& b)
{
    const double da = QLineF(center, a).length();
    const double db = QLineF(center, b).length();
    if (da < 1e-6) return b;
    if (db < 1e-6) return a;
    return da >= db ? a : b;
}

static ParsedDimensionText parseDimensionText(const QString& rawText)
{
    ParsedDimensionText parsed;
    const QString trimmed = rawText.trimmed();
    if (trimmed.isEmpty() || trimmed == "<>") {
        return parsed;
    }

    QString normalized = trimmed;
    normalized.replace("%%c", QString::fromUtf8("Ø"));

    if (normalized.startsWith(QString::fromUtf8("Ø<>"))) {
        parsed.prefix = QString::fromUtf8("Ø");
        return parsed;
    }
    if (normalized.startsWith("R<>")) {
        parsed.prefix = "R";
        return parsed;
    }
    if (normalized.contains("<>")) {
        return parsed;
    }

    parsed.customText = normalized;
    return parsed;
}

static QString metaValue(const DxfMeta& meta, const QString& key, const QString& fallback = QString())
{
    return meta.values.value(key, fallback);
}

static int metaInt(const DxfMeta& meta, const QString& key, int fallback = 0)
{
    bool ok = false;
    const int value = meta.values.value(key).toInt(&ok);
    return ok ? value : fallback;
}

static double metaDouble(const DxfMeta& meta, const QString& key, double fallback = 0.0)
{
    bool ok = false;
    const double value = meta.values.value(key).toDouble(&ok);
    return ok ? value : fallback;
}

static bool metaBool(const DxfMeta& meta, const QString& key, bool fallback = false)
{
    if (!meta.values.contains(key)) return fallback;
    const QString value = meta.values.value(key).trimmed().toLower();
    return value == "1" || value == "true" || value == "yes";
}

static PrimitiveType primitiveTypeFromToken(const QString& token)
{
    const QString upper = token.trimmed().toUpper();
    if (upper == "SEGMENT") return PrimitiveType::Segment;
    if (upper == "CIRCLE") return PrimitiveType::Circle;
    if (upper == "ARC") return PrimitiveType::Arc;
    if (upper == "RECTANGLE") return PrimitiveType::Rectangle;
    if (upper == "ELLIPSE") return PrimitiveType::Ellipse;
    if (upper == "POLYGON") return PrimitiveType::Polygon;
    if (upper == "SPLINE") return PrimitiveType::Spline;
    if (upper == "LINEAR_DIMENSION") return PrimitiveType::LinearDimension;
    if (upper == "RADIAL_DIMENSION") return PrimitiveType::RadialDimension;
    if (upper == "ANGULAR_DIMENSION") return PrimitiveType::AngularDimension;
    if (upper == "POINT") return PrimitiveType::Point;
    return PrimitiveType::Generic;
}

static SnapType snapTypeFromToken(const QString& token)
{
    const QString upper = token.trimmed().toUpper();
    if (upper == "ENDPOINT") return SnapType::Endpoint;
    if (upper == "MIDPOINT") return SnapType::Midpoint;
    if (upper == "CENTER") return SnapType::Center;
    if (upper == "INTERSECTION") return SnapType::Intersection;
    if (upper == "PERPENDICULAR") return SnapType::Perpendicular;
    if (upper == "TANGENT") return SnapType::Tangent;
    if (upper == "QUADRANT") return SnapType::Quadrant;
    if (upper == "GRID") return SnapType::Grid;
    if (upper == "NEAREST") return SnapType::Nearest;
    if (upper == "ALL") return SnapType::All;
    return SnapType::None;
}

static LinearDimensionMode linearModeFromToken(const QString& token)
{
    const QString upper = token.trimmed().toUpper();
    if (upper == "HORIZONTAL") return LinearDimensionMode::Horizontal;
    if (upper == "VERTICAL") return LinearDimensionMode::Vertical;
    return LinearDimensionMode::Aligned;
}

static DimensionStyle extractDimensionStyle(const DxfMeta& meta, const DimensionStyle& baseStyle)
{
    DimensionStyle style = baseStyle;
    if (meta.values.contains("CLARUSCAD_DIM_FONT")) style.fontFamily = metaValue(meta, "CLARUSCAD_DIM_FONT");
    if (meta.values.contains("CLARUSCAD_DIM_TEXT_HEIGHT")) style.textHeight = metaDouble(meta, "CLARUSCAD_DIM_TEXT_HEIGHT", style.textHeight);
    if (meta.values.contains("CLARUSCAD_DIM_TEXT_GAP")) style.textGap = metaDouble(meta, "CLARUSCAD_DIM_TEXT_GAP", style.textGap);
    if (meta.values.contains("CLARUSCAD_DIM_TEXT_ALONG")) style.textAlongLineOffset = metaDouble(meta, "CLARUSCAD_DIM_TEXT_ALONG", style.textAlongLineOffset);
    if (meta.values.contains("CLARUSCAD_DIM_ARROW_SIZE")) style.arrowSize = metaDouble(meta, "CLARUSCAD_DIM_ARROW_SIZE", style.arrowSize);
    if (meta.values.contains("CLARUSCAD_DIM_ARROW_TYPE")) style.arrowType = static_cast<DimensionArrowType>(metaInt(meta, "CLARUSCAD_DIM_ARROW_TYPE", static_cast<int>(style.arrowType)));
    if (meta.values.contains("CLARUSCAD_DIM_ARROW_FILLED")) style.arrowFilled = metaBool(meta, "CLARUSCAD_DIM_ARROW_FILLED", style.arrowFilled);
    if (meta.values.contains("CLARUSCAD_DIM_EXT_OFFSET")) style.extensionLineOffset = metaDouble(meta, "CLARUSCAD_DIM_EXT_OFFSET", style.extensionLineOffset);
    if (meta.values.contains("CLARUSCAD_DIM_EXT_EXTEND")) style.extensionLineExtend = metaDouble(meta, "CLARUSCAD_DIM_EXT_EXTEND", style.extensionLineExtend);
    if (meta.values.contains("CLARUSCAD_DIM_LINE_EXT")) style.dimensionLineExtension = metaDouble(meta, "CLARUSCAD_DIM_LINE_EXT", style.dimensionLineExtension);
    if (meta.values.contains("CLARUSCAD_DIM_EXT_LTYPE")) style.extensionLineTypeId = metaInt(meta, "CLARUSCAD_DIM_EXT_LTYPE", style.extensionLineTypeId);
    if (meta.values.contains("CLARUSCAD_DIM_LINE_LTYPE")) style.dimensionLineTypeId = metaInt(meta, "CLARUSCAD_DIM_LINE_LTYPE", style.dimensionLineTypeId);
    if (meta.values.contains("CLARUSCAD_DIM_TEXT_COLOR")) style.textColor = QColor::fromRgba(static_cast<QRgb>(metaInt(meta, "CLARUSCAD_DIM_TEXT_COLOR", style.textColor.rgba())));
    if (meta.values.contains("CLARUSCAD_DIM_EXT_COLOR")) style.extensionLineColor = QColor::fromRgba(static_cast<QRgb>(metaInt(meta, "CLARUSCAD_DIM_EXT_COLOR", style.extensionLineColor.rgba())));
    if (meta.values.contains("CLARUSCAD_DIM_LINE_COLOR")) style.dimensionLineColor = QColor::fromRgba(static_cast<QRgb>(metaInt(meta, "CLARUSCAD_DIM_LINE_COLOR", style.dimensionLineColor.rgba())));
    return style;
}

static void applyDimensionMeta(BaseDimensionPrimitive* dim, const DxfMeta& meta, const DimensionStyle& baseStyle)
{
    dim->setStyle(extractDimensionStyle(meta, baseStyle));
    dim->setCustomText(metaValue(meta, "CLARUSCAD_DIM_CUSTOM_TEXT"));
    dim->setShelfEnabled(metaBool(meta, "CLARUSCAD_DIM_HAS_SHELF", false));
    if (metaBool(meta, "CLARUSCAD_DIM_HAS_CUSTOM_TEXT_POS", false)) {
        dim->setCustomTextPosition(QPointF(metaDouble(meta, "CLARUSCAD_DIM_TEXT_POS_X"), metaDouble(meta, "CLARUSCAD_DIM_TEXT_POS_Y")));
    }
}

static LinearDimensionPrimitive::Attachment loadAttachment(const DxfMeta& meta, const QString& prefix,
                                                           const DimensionAssociationMap& associations)
{
    LinearDimensionPrimitive::Attachment attachment;
    attachment.snapType = snapTypeFromToken(metaValue(meta, QString("CLARUSCAD_%1_SNAP").arg(prefix)));
    attachment.index = metaInt(meta, QString("CLARUSCAD_%1_INDEX").arg(prefix), -1);
    attachment.param = metaDouble(meta, QString("CLARUSCAD_%1_PARAM").arg(prefix), 0.0);
    attachment.fallback = QPointF(metaDouble(meta, QString("CLARUSCAD_%1_FALLBACK_X").arg(prefix), 0.0),
                                  metaDouble(meta, QString("CLARUSCAD_%1_FALLBACK_Y").arg(prefix), 0.0));
    const int sourceId = metaInt(meta, QString("CLARUSCAD_%1_SOURCE").arg(prefix), -1);
    if (sourceId >= 0 && associations.byId.contains(sourceId)) {
        const ImportedEntityRef ref = associations.byId.value(sourceId);
        BasePrimitive* source = ref.primary;
        bool resolved = true;
        if (!ref.edges.isEmpty()) {
            const PrimitiveType sourceType = primitiveTypeFromToken(metaValue(meta, QString("CLARUSCAD_%1_SOURCE_TYPE").arg(prefix)));
            if (sourceType == PrimitiveType::Rectangle || sourceType == PrimitiveType::Polygon) {
                if ((attachment.snapType == SnapType::Endpoint
                     || attachment.snapType == SnapType::Midpoint
                     || attachment.snapType == SnapType::Nearest)
                    && attachment.index >= 0 && attachment.index < ref.edges.size()) {
                    source = ref.edges[attachment.index];
                    attachment.index = 0;
                } else {
                    resolved = false;
                }
            }
        }
        attachment.source = resolved ? source : nullptr;
    }
    return attachment;
}

static void registerImportedPrimitive(const DxfMeta& meta, BasePrimitive* primitive,
                                      DimensionAssociationMap& associations)
{
    const int clarusId = metaInt(meta, "CLARUSCAD_ID", -1);
    if (clarusId < 0 || !primitive) return;
    ImportedEntityRef ref;
    ref.primary = primitive;
    ref.edges.append(primitive);
    associations.byId.insert(clarusId, ref);
}

static void registerImportedEdges(const DxfMeta& meta, const QVector<BasePrimitive*>& edges,
                                  DimensionAssociationMap& associations)
{
    const int clarusId = metaInt(meta, "CLARUSCAD_ID", -1);
    if (clarusId < 0 || edges.isEmpty()) return;
    ImportedEntityRef ref;
    ref.primary = edges.first();
    ref.edges = edges;
    associations.byId.insert(clarusId, ref);
}

// Прямая обработка одной сущности — добавляет примитивы в сцену
// Возвращает true, если обработана
static void processEntity(const DxfEntity& entity, Scene& scene,
                           const QMap<QString, DxfBlock>& blocks,
                           const QHash<QString, DimensionStyle>& dimStyles,
                           DimensionAssociationMap& associations,
                           int depth = 0);

// Обработка полилинии из набора вершин
static QVector<BasePrimitive*> createPolylineSegments(Scene& scene, const QVector<QPointF>& vertices,
                                     bool closed, const EntityProps& props) {
    QVector<BasePrimitive*> created;
    if (vertices.size() < 2) return created;
    for (int i = 0; i < vertices.size() - 1; ++i) {
        auto* seg = new SegmentPrimitive(
            PointPrimitive(vertices[i].x(), vertices[i].y()),
            PointPrimitive(vertices[i+1].x(), vertices[i+1].y()));
        applyProps(seg, props);
        created.append(seg);
        scene.addPrimitive(std::unique_ptr<BasePrimitive>(seg));
    }
    if (closed && vertices.size() > 2) {
        auto* seg = new SegmentPrimitive(
            PointPrimitive(vertices.last().x(), vertices.last().y()),
            PointPrimitive(vertices.first().x(), vertices.first().y()));
        applyProps(seg, props);
        created.append(seg);
        scene.addPrimitive(std::unique_ptr<BasePrimitive>(seg));
    }
    return created;
}

static void processEntity(const DxfEntity& entity, Scene& scene,
                           const QMap<QString, DxfBlock>& blocks,
                           const QHash<QString, DimensionStyle>& dimStyles,
                           DimensionAssociationMap& associations,
                           int depth) {
    // Защита от бесконечной рекурсии (максимум 10 уровней вложенности)
    if (depth > 10) return;

    const auto& data = entity.data;
    EntityProps props = extractCommonProps(data);
    const DxfMeta meta = extractMeta(data);

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
        registerImportedPrimitive(meta, seg, associations);
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
            registerImportedPrimitive(meta, circ, associations);
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
            registerImportedPrimitive(meta, arc, associations);
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
        // Обратная конвертация DXF → Qt:
        // Экспортёр инвертировал dy (Y-flip), поэтому для восстановления
        // Qt-угла: rotation = atan2(-dy, dx)
        // Всегда создаём с rx=majorRadius, ry=minorRadius.
        double rotation = std::atan2(-dy, dx) * 180.0 / M_PI;
        if (majorRadius > 0) {
            auto* ell = new EllipsePrimitive(PointPrimitive(cx, cy), majorRadius, minorRadius, rotation);
            applyProps(ell, props);
            scene.addPrimitive(std::unique_ptr<BasePrimitive>(ell));
            registerImportedPrimitive(meta, ell, associations);
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
            registerImportedPrimitive(meta, spline, associations);
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
        const QVector<BasePrimitive*> edges = createPolylineSegments(scene, pts, closed, props);
        registerImportedEdges(meta, edges, associations);
    }
    else if (entity.type == "DIMENSION") {
        QString kind = metaValue(meta, "CLARUSCAD_DIM_KIND").trimmed().toUpper();
        const int dimType = dataInt(data, 70, 0) & 0x0F;
        const QString styleName = dataValue(data, 3).trimmed();
        const DimensionStyle baseStyle = dimStyles.value(styleName, SettingsManager::instance().getDefaultDimensionStyle());
        const ParsedDimensionText parsedText = parseDimensionText(dataValue(data, 1));
        if (kind.isEmpty()) {
            if (dimType == 0 || dimType == 1) kind = "LINEAR";
            else if (dimType == 3) kind = "DIAMETER";
            else if (dimType == 4) kind = "RADIUS";
            else if (dimType == 2 || dimType == 5) kind = "ANGULAR";
        }
        if (kind == "LINEAR") {
            auto* dim = new LinearDimensionPrimitive();
            QPointF start(metaDouble(meta, "CLARUSCAD_DIM_START_FALLBACK_X"), metaDouble(meta, "CLARUSCAD_DIM_START_FALLBACK_Y"));
            QPointF end(metaDouble(meta, "CLARUSCAD_DIM_END_FALLBACK_X"), metaDouble(meta, "CLARUSCAD_DIM_END_FALLBACK_Y"));
            QPointF textAnchor;
            bool hasTextAnchor = false;
            for (const auto& d : data) {
                if (d.code == 13) start.setX(d.value.toDouble());
                if (d.code == 23) start.setY(d.value.toDouble());
                if (d.code == 14) end.setX(d.value.toDouble());
                if (d.code == 24) end.setY(d.value.toDouble());
                if (d.code == 11) { textAnchor.setX(d.value.toDouble()); hasTextAnchor = true; }
                if (d.code == 21) { textAnchor.setY(d.value.toDouble()); hasTextAnchor = true; }
            }
            dim->setStartPoint(start);
            dim->setEndPoint(end);
            dim->setDimensionLinePos(QPointF(0.0, 0.0));
            LinearDimensionMode mode = linearModeFromToken(metaValue(meta, "CLARUSCAD_DIM_MODE"));
            if (!meta.values.contains("CLARUSCAD_DIM_MODE")) {
                if (dimType == 0) {
                    const double angle = normalizeAngleDegrees(dataDouble(data, 50, 0.0));
                    if (std::abs(angle - 90.0) < 1e-3 || std::abs(angle - 270.0) < 1e-3) mode = LinearDimensionMode::Vertical;
                    else if (std::abs(angle) < 1e-3 || std::abs(angle - 180.0) < 1e-3) mode = LinearDimensionMode::Horizontal;
                    else mode = LinearDimensionMode::Aligned;
                } else {
                    mode = LinearDimensionMode::Aligned;
                }
            }
            dim->setMode(mode);
            dim->setStartAttachment(loadAttachment(meta, "DIM_START", associations));
            dim->setEndAttachment(loadAttachment(meta, "DIM_END", associations));
            dim->updateFromAttachments();

            QPointF linePos;
            bool hasLineMeta = meta.values.contains("CLARUSCAD_DIM_LINE_X") && meta.values.contains("CLARUSCAD_DIM_LINE_Y");
            if (hasLineMeta) {
                linePos = QPointF(metaDouble(meta, "CLARUSCAD_DIM_LINE_X"), metaDouble(meta, "CLARUSCAD_DIM_LINE_Y"));
            } else {
                linePos = dataPoint(data, 10, 20);
            }
            dim->setDimensionLinePos(linePos);
            applyProps(dim, props);
            applyDimensionMeta(dim, meta, baseStyle);
            dim->setTextPrefix(meta.values.contains("CLARUSCAD_DIM_PREFIX") ? metaValue(meta, "CLARUSCAD_DIM_PREFIX")
                                                                            : parsedText.prefix);
            if (!meta.values.contains("CLARUSCAD_DIM_CUSTOM_TEXT") && !parsedText.customText.isEmpty()) {
                dim->setCustomText(parsedText.customText);
            }
            if (!metaBool(meta, "CLARUSCAD_DIM_HAS_CUSTOM_TEXT_POS", false) && hasTextAnchor) {
                dim->setCustomTextPosition(textAnchor);
            }
            dim->recalculateValue();
            scene.addPrimitive(std::unique_ptr<BasePrimitive>(dim));
            registerImportedPrimitive(meta, dim, associations);
        }
        else if (kind == "RADIUS" || kind == "DIAMETER") {
            auto* dim = new RadialDimensionPrimitive();
            QPointF center(metaDouble(meta, "CLARUSCAD_DIM_CENTER_X"), metaDouble(meta, "CLARUSCAD_DIM_CENTER_Y"));
            QPointF radiusPoint(metaDouble(meta, "CLARUSCAD_DIM_RADIUS_X"), metaDouble(meta, "CLARUSCAD_DIM_RADIUS_Y"));
            QPointF linePos(metaDouble(meta, "CLARUSCAD_DIM_LINE_X"), metaDouble(meta, "CLARUSCAD_DIM_LINE_Y"));
            QPointF textAnchor;
            bool hasTextAnchor = false;
            if (!meta.values.contains("CLARUSCAD_DIM_CENTER_X") || !meta.values.contains("CLARUSCAD_DIM_RADIUS_X")) {
                const QPointF code10 = dataPoint(data, 10, 20);
                const QPointF code11 = dataPoint(data, 11, 21);
                const QPointF code15 = dataPoint(data, 15, 25);
                hasTextAnchor = dataHasCode(data, 11) || dataHasCode(data, 21);
                textAnchor = code11;

                if (kind == "DIAMETER") {
                    center = (code10 + code15) * 0.5;
                    radiusPoint = code15;
                } else {
                    center = code10;
                    radiusPoint = code15;
                }
                linePos = hasTextAnchor ? code11 : radiusPoint;
            }
            dim->setCenterPoint(center);
            dim->setRadiusPoint(radiusPoint);
            dim->setDiameterMode(kind == "DIAMETER");
            dim->setDimensionLinePos(linePos);

            const int sourceId = metaInt(meta, "CLARUSCAD_DIM_ASSOC_SOURCE", -1);
            if (sourceId >= 0 && associations.byId.contains(sourceId)) {
                dim->setAssociatedPrimitive(associations.byId.value(sourceId).primary,
                                            metaDouble(meta, "CLARUSCAD_DIM_ASSOC_ANGLE", 0.0));
                dim->updateFromAssociation();
            }

            applyProps(dim, props);
            applyDimensionMeta(dim, meta, baseStyle);
            if (!meta.values.contains("CLARUSCAD_DIM_CUSTOM_TEXT") && !parsedText.customText.isEmpty()) {
                dim->setCustomText(parsedText.customText);
            }
            if (!metaBool(meta, "CLARUSCAD_DIM_HAS_CUSTOM_TEXT_POS", false) && hasTextAnchor) {
                dim->setCustomTextPosition(textAnchor);
            }
            dim->recalculateValue();
            scene.addPrimitive(std::unique_ptr<BasePrimitive>(dim));
            registerImportedPrimitive(meta, dim, associations);
        }
        else if (kind == "ANGULAR") {
            auto* dim = new AngularDimensionPrimitive();
            QPointF center(metaDouble(meta, "CLARUSCAD_DIM_CENTER_X"), metaDouble(meta, "CLARUSCAD_DIM_CENTER_Y"));
            QPointF start(metaDouble(meta, "CLARUSCAD_DIM_START_X"), metaDouble(meta, "CLARUSCAD_DIM_START_Y"));
            QPointF end(metaDouble(meta, "CLARUSCAD_DIM_END_X"), metaDouble(meta, "CLARUSCAD_DIM_END_Y"));
            QPointF arcPoint(metaDouble(meta, "CLARUSCAD_DIM_ARC_X"), metaDouble(meta, "CLARUSCAD_DIM_ARC_Y"));
            QPointF textAnchor;
            bool hasTextAnchor = false;
            if (!meta.values.contains("CLARUSCAD_DIM_CENTER_X")) {
                hasTextAnchor = dataHasCode(data, 11) || dataHasCode(data, 21);
                textAnchor = dataPoint(data, 11, 21);
                if (hasSubclassMarker(data, "AcDb2LineAngularDimension") || dimType == 2) {
                    const QPointF line1a = dataPoint(data, 13, 23);
                    const QPointF line1b = dataPoint(data, 14, 24);
                    const QPointF line2a = dataPoint(data, 10, 20);
                    const QPointF line2b = dataPoint(data, 15, 25);
                    if (!lineIntersectionPoint(line1a, line1b, line2a, line2b, center)) {
                        center = line1a;
                    }
                    start = fartherRayPoint(center, line1a, line1b);
                    end = fartherRayPoint(center, line2a, line2b);
                    arcPoint = dataPoint(data, 16, 26);
                } else {
                    start = dataPoint(data, 13, 23);
                    end = dataPoint(data, 14, 24);
                    center = dataPoint(data, 15, 25);
                    arcPoint = dataPoint(data, 10, 20);
                }
            }
            dim->setCenterPoint(center);
            dim->setStartPoint(start);
            dim->setEndPoint(end);
            dim->setArcPoint(arcPoint);

            const int firstSourceId = metaInt(meta, "CLARUSCAD_DIM_FIRST_SOURCE", -1);
            const int secondSourceId = metaInt(meta, "CLARUSCAD_DIM_SECOND_SOURCE", -1);
            if (firstSourceId >= 0 && secondSourceId >= 0
                && associations.byId.contains(firstSourceId)
                && associations.byId.contains(secondSourceId)) {
                ImportedEntityRef firstRef = associations.byId.value(firstSourceId);
                ImportedEntityRef secondRef = associations.byId.value(secondSourceId);
                const int firstEdge = metaInt(meta, "CLARUSCAD_DIM_FIRST_EDGE", 0);
                const int secondEdge = metaInt(meta, "CLARUSCAD_DIM_SECOND_EDGE", 0);
                BasePrimitive* firstSource = !firstRef.edges.isEmpty() && firstEdge >= 0 && firstEdge < firstRef.edges.size()
                    ? firstRef.edges[firstEdge]
                    : firstRef.primary;
                BasePrimitive* secondSource = !secondRef.edges.isEmpty() && secondEdge >= 0 && secondEdge < secondRef.edges.size()
                    ? secondRef.edges[secondEdge]
                    : secondRef.primary;
                dim->setEdgeAssociation(firstSource, 0, secondSource, 0);
                dim->updateFromAssociation();
                dim->setArcPoint(arcPoint);
            }

            applyProps(dim, props);
            applyDimensionMeta(dim, meta, baseStyle);
            if (!meta.values.contains("CLARUSCAD_DIM_CUSTOM_TEXT") && !parsedText.customText.isEmpty()) {
                dim->setCustomText(parsedText.customText);
            }
            if (!metaBool(meta, "CLARUSCAD_DIM_HAS_CUSTOM_TEXT_POS", false) && hasTextAnchor) {
                dim->setCustomTextPosition(textAnchor);
            }
            dim->recalculateValue();
            scene.addPrimitive(std::unique_ptr<BasePrimitive>(dim));
            registerImportedPrimitive(meta, dim, associations);
        }
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
                processEntity(blockEntity, scene, blocks, dimStyles, associations, depth + 1);
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
    DimensionAssociationMap associations;

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

    QHash<QString, QString> textStyleFontsByHandle;
    QHash<QString, QString> blockRecordNamesByHandle;
    QHash<QString, DimensionStyle> dimStyles;

    for (int scan = 0; scan < totalPairs; ++scan) {
        if (pairs[scan].code != 0) continue;

        const QString type = pairs[scan].value;
        if (type != "STYLE" && type != "BLOCK_RECORD" && type != "DIMSTYLE") continue;

        auto [recordData, nextPos] = collectEntityData(scan + 1);
        scan = nextPos - 1;

        if (type == "STYLE") {
            if (!hasSubclassMarker(recordData, "AcDbTextStyleTableRecord")) continue;
            const QString handle = dataValue(recordData, 5).trimmed();
            const QString fontToken = dataValue(recordData, 3).trimmed();
            const QString styleName = dataValue(recordData, 2).trimmed();
            QString fontFamily = QFileInfo(fontToken).completeBaseName();
            if (fontFamily.isEmpty()) fontFamily = fontToken;
            if (fontFamily.isEmpty()) fontFamily = styleName;
            if (!handle.isEmpty() && !fontFamily.isEmpty()) {
                textStyleFontsByHandle.insert(handle, fontFamily);
            }
        } else if (type == "BLOCK_RECORD") {
            if (!hasSubclassMarker(recordData, "AcDbBlockTableRecord")) continue;
            const QString handle = dataValue(recordData, 5).trimmed();
            const QString blockName = dataValue(recordData, 2).trimmed();
            if (!handle.isEmpty() && !blockName.isEmpty()) {
                blockRecordNamesByHandle.insert(handle, blockName);
            }
        } else if (type == "DIMSTYLE") {
            if (!hasSubclassMarker(recordData, "AcDbDimStyleTableRecord")) continue;
            const QString styleName = dataValue(recordData, 2).trimmed();
            if (styleName.isEmpty()) continue;

            DimensionStyle style = SettingsManager::instance().getDefaultDimensionStyle();
            if (dataHasCode(recordData, 140)) style.textHeight = dataDouble(recordData, 140, style.textHeight);
            if (dataHasCode(recordData, 147)) style.textGap = dataDouble(recordData, 147, style.textGap);
            if (dataHasCode(recordData, 41)) style.arrowSize = dataDouble(recordData, 41, style.arrowSize);
            if (dataHasCode(recordData, 42)) style.extensionLineOffset = dataDouble(recordData, 42, style.extensionLineOffset);
            if (dataHasCode(recordData, 44)) style.extensionLineExtend = dataDouble(recordData, 44, style.extensionLineExtend);

            const QString textStyleHandle = dataValue(recordData, 340).trimmed();
            if (!textStyleHandle.isEmpty() && textStyleFontsByHandle.contains(textStyleHandle)) {
                style.fontFamily = textStyleFontsByHandle.value(textStyleHandle);
            }

            const int dimColor = dataHasCode(recordData, 284) ? dataInt(recordData, 284, -1) : dataInt(recordData, 176, -1);
            const int extColor = dataHasCode(recordData, 285) ? dataInt(recordData, 285, -1) : dataInt(recordData, 177, -1);
            const int textColor = dataHasCode(recordData, 286) ? dataInt(recordData, 286, -1) : dataInt(recordData, 178, -1);
            if (dimColor > 0 && dimColor != 256) style.dimensionLineColor = getQColorFromAutoCadIndex(dimColor);
            if (extColor > 0 && extColor != 256) style.extensionLineColor = getQColorFromAutoCadIndex(extColor);
            if (textColor > 0 && textColor != 256) style.textColor = getQColorFromAutoCadIndex(textColor);

            const QString arrowBlock = blockRecordNamesByHandle.value(dataValue(recordData, 342).trimmed());
            if (!arrowBlock.isEmpty() && arrowBlock.contains("Arrow", Qt::CaseInsensitive)) {
                style.arrowType = DimensionArrowType::ClosedFilled;
                style.arrowFilled = true;
            }

            dimStyles.insert(styleName, style);
        }
    }

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
                processEntity(polyEntity, scene, blocks, dimStyles, associations);
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
            processEntity(polyEntity, scene, blocks, dimStyles, associations);
            readingPolyline = false;
            currentPolyVertices.clear();
        }
        else if (!readingPolyline) {
            DxfEntity ent;
            ent.type = entityType;
            ent.data = entData;
            processEntity(ent, scene, blocks, dimStyles, associations);
        }
    }

    return true;
}
