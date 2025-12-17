#include "ObjectBindingManager.h"
#include "Scene.h"
#include "BasePrimitive.h"
#include <QtMath>
#include <limits>

ObjectBindingManager& ObjectBindingManager::instance()
{
    static ObjectBindingManager manager;
    return manager;
}

ObjectBindingManager::ObjectBindingManager(QObject* parent) : QObject(parent) {}

void ObjectBindingManager::setGridSnap(bool enabled) { m_gridSnapEnabled = enabled; }
bool ObjectBindingManager::isGridSnapEnabled() const { return m_gridSnapEnabled; }

void ObjectBindingManager::setPrimitiveSnap(bool enabled) { m_primitiveSnapEnabled = enabled; }
bool ObjectBindingManager::isPrimitiveSnapEnabled() const { return m_primitiveSnapEnabled; }

void ObjectBindingManager::setGridStep(int step) { m_gridStep = step; }
int ObjectBindingManager::getGridStep() const { return m_gridStep; }

double ObjectBindingManager::calculateDynamicGridStep(double zoomFactor) const
{
    //вычисление визуального шага
    double visualGridStep = m_gridStep * zoomFactor;
    double dynamicGridStep = m_gridStep;

    //корректировка шага (адаптивная сетка)
    while (visualGridStep < 15) {
        dynamicGridStep *= 2;
        visualGridStep *= 2;
    }
    while (visualGridStep > 150) {
        dynamicGridStep /= 2;
        visualGridStep /= 2;
    }
    return dynamicGridStep;
}

QPointF ObjectBindingManager::getSnappedPoint(const QPointF& currentPos, const Scene* scene, double zoomFactor, BasePrimitive* excludePrimitive) const
{
    QPointF result = currentPos;
    bool snapped = false;

    //1. Привязка к объектам (имеет приоритет выше сетки)
    if (m_primitiveSnapEnabled && scene) {
        QPointF objSnap = snapToPrimitives(currentPos, scene, zoomFactor, excludePrimitive);
        //если точка изменилась, значит привязка сработала
        if (objSnap != currentPos) {
            result = objSnap;
            snapped = true;
        }
    }

    //2. Привязка к сетке (если не привязались к объекту)
    if (!snapped && m_gridSnapEnabled) {
        result = snapToGrid(currentPos, zoomFactor);
    }

    return result;
}

QPointF ObjectBindingManager::snapToGrid(const QPointF& pos, double zoomFactor) const
{
    double step = calculateDynamicGridStep(zoomFactor);
    if (step <= 0) return pos;

    //округление до ближайшего узла сетки
    double snappedX = std::round(pos.x() / step) * step;
    double snappedY = std::round(pos.y() / step) * step;

    return QPointF(snappedX, snappedY);
}

QPointF ObjectBindingManager::snapToPrimitives(const QPointF& pos, const Scene* scene, double zoomFactor, BasePrimitive* excludePrimitive) const
{
    double minDistance = 15.0 / zoomFactor; //радиус магнита в пикселях экрана
    QPointF bestPoint = pos;
    double currentMinDist = std::numeric_limits<double>::max();

    for (const auto& prim : scene->getPrimitives()) {
        if (prim.get() == excludePrimitive) continue;

        //получаем точки привязки от самого примитива (Smart Model)
        QVector<QPointF> snapPoints = prim->getSnapPoints();

        for (const QPointF& pt : snapPoints) {
            double dist = QLineF(pos, pt).length();

            //ищем самую близкую точку среди всех примитивов
            if (dist < minDistance && dist < currentMinDist) {
                currentMinDist = dist;
                bestPoint = pt;
            }
        }
    }
    return bestPoint;
}
