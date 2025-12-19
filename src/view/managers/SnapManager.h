//SnapManager - менеджер расширенных привязок к объектам
//Поддерживает: конец, середина, центр, пересечение, перпендикуляр, касательная, квадранты

#pragma once

#include <QPointF>
#include <QVector>
#include <QObject>

class BasePrimitive;
class Scene;

// Типы привязок
enum class SnapType {
    None = 0,
    Endpoint = 1 << 0,      // Концы отрезков, дуг
    Midpoint = 1 << 1,      // Середина отрезка/дуги
    Center = 1 << 2,        // Центр окружности/дуги/эллипса
    Intersection = 1 << 3, // Пересечения примитивов
    Perpendicular = 1 << 4,// Перпендикуляр к линии
    Tangent = 1 << 5,      // Касательная к окружности/дуге
    Quadrant = 1 << 6,     // Квадранты окружности (0°, 90°, 180°, 270°)
    Grid = 1 << 7,         // Привязка к сетке
    All = 0xFF             // Все типы
};

// Структура точки привязки
struct SnapPoint {
    QPointF position;      // Координаты точки
    SnapType type;         // Тип привязки
    BasePrimitive* source = nullptr; // Источник привязки (примитив)
    double distance = 0.0; // Расстояние до курсора (для сортировки)
    
    // Для сравнения при сортировке
    bool operator<(const SnapPoint& other) const {
        return distance < other.distance;
    }
};

// Менеджер привязок (синглтон)
class SnapManager : public QObject
{
    Q_OBJECT

public:
    static SnapManager& instance();
    
    // Поиск ближайшей точки привязки
    SnapPoint findNearestSnapPoint(const QPointF& mousePos, Scene* scene, 
                                    double zoomFactor, double tolerance = 15.0);
    
    // Получить все точки привязки в радиусе
    QVector<SnapPoint> findSnapPointsInRadius(const QPointF& mousePos, Scene* scene,
                                               double zoomFactor, double radius = 30.0);
    
    // Управление включенными типами привязок
    void setSnapTypesEnabled(int snapFlags);
    int getSnapTypesEnabled() const;
    
    void setSnapTypeEnabled(SnapType type, bool enabled);
    bool isSnapTypeEnabled(SnapType type) const;
    
    // Привязка к сетке
    void setGridStep(double step);
    double getGridStep() const;

signals:
    void snapTypesChanged();

private:
    SnapManager();
    ~SnapManager() = default;
    
    // Сбор точек привязки от отдельных примитивов
    void collectSnapPoints(const QPointF& mousePos, BasePrimitive* primitive, 
                           double tolerance, QVector<SnapPoint>& outPoints);
    
    // Вспомогательные методы для разных типов привязок
    void collectEndpoints(BasePrimitive* prim, QVector<SnapPoint>& out);
    void collectMidpoints(BasePrimitive* prim, QVector<SnapPoint>& out);
    void collectCenters(BasePrimitive* prim, QVector<SnapPoint>& out);
    void collectQuadrants(BasePrimitive* prim, QVector<SnapPoint>& out);
    
    // Сбор точек пересечения между примитивами
    void collectIntersections(const QPointF& mousePos, Scene* scene, 
                               double tolerance, QVector<SnapPoint>& out);
    
    // Сбор точек перпендикуляра от текущей позиции к примитивам
    void collectPerpendiculars(const QPointF& mousePos, const QPointF& basePoint,
                                Scene* scene, double tolerance, QVector<SnapPoint>& out);
    
    // Сбор точек касательной к окружностям/дугам
    void collectTangents(const QPointF& mousePos, const QPointF& basePoint,
                          Scene* scene, double tolerance, QVector<SnapPoint>& out);
    
    // Вспомогательные геометрические функции
    QVector<QPointF> findSegmentSegmentIntersection(const QPointF& a1, const QPointF& a2,
                                                     const QPointF& b1, const QPointF& b2);
    QVector<QPointF> findSegmentCircleIntersection(const QPointF& p1, const QPointF& p2,
                                                    const QPointF& center, double radius);
    QVector<QPointF> findCircleCircleIntersection(const QPointF& c1, double r1,
                                                   const QPointF& c2, double r2);
    QPointF findPerpendicularToSegment(const QPointF& from, const QPointF& segStart, 
                                        const QPointF& segEnd);
    QVector<QPointF> findTangentPointsToCircle(const QPointF& from, const QPointF& center, 
                                                double radius);
    QVector<QPointF> findTangentPointsToEllipse(const QPointF& from, const QPointF& center,
                                                 double rx, double ry, double rotation);
    
    // Привязка к сетке
    SnapPoint snapToGrid(const QPointF& pos);

    int m_enabledSnapTypes = static_cast<int>(SnapType::All);
    double m_gridStep = 50.0;
    
    // Базовая точка для привязок перпендикуляр/касательная (первая точка при построении)
    QPointF m_basePoint;
    bool m_hasBasePoint = false;
    
public:
    // Установка базовой точки для привязок (вызывается при первом клике)
    void setBasePoint(const QPointF& point) { m_basePoint = point; m_hasBasePoint = true; }
    void clearBasePoint() { m_hasBasePoint = false; }
};
