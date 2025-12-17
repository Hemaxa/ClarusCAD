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
    
    // Привязка к сетке
    SnapPoint snapToGrid(const QPointF& pos);

    int m_enabledSnapTypes = static_cast<int>(SnapType::All);
    double m_gridStep = 50.0;
};
