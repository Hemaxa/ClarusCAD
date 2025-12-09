#include "DeleteTool.h"
#include "Scene.h"
#include "ViewportPanelWidget.h"
#include "SegmentPrimitive.h"
#include "CirclePrimitive.h" // Подключаем окружность

#include <QApplication>
#include <QCursor>
#include <QMouseEvent>
#include <QtMath>

DeleteTool::DeleteTool(QObject* parent) : BaseCreationTool(parent) {}

void DeleteTool::onMousePress(QMouseEvent* event, Scene* scene, ViewportPanelWidget* viewport)
{
    //проверка на то, что нажата ЛКМ
    if (event->button() != Qt::LeftButton) {
        return;
    }

    //получение координат мировых клика
    QPointF clickPos = event->position();
    BasePrimitive* primitiveToHit = nullptr;
    double minDistance = 10.0 / viewport->getZoomFactor();  //порог попадания

    //определение ближайшего к клику примитива
    for (const auto& primitive : scene->getPrimitives()) {
        double distance = std::numeric_limits<double>::max();

        //если примитив - "Отрезок"
        if (primitive->getType() == PrimitiveType::Segment) {
            auto* segment = static_cast<SegmentPrimitive*>(primitive.get());
            QPointF p1(segment->getStart().getX(), segment->getStart().getY());
            QPointF p2(segment->getEnd().getX(), segment->getEnd().getY());
            QLineF line(p1, p2);

            //вектор отрезка
            QPointF lineVec = p2 - p1;

            //вектор от начала отрезка до точки клика
            QPointF pointVec = clickPos - p1;

            double lineLen2 = QPointF::dotProduct(lineVec, lineVec);

            if (lineLen2 == 0.0) {
                distance = QLineF(clickPos, p1).length();
            } else {
                //проекция точки на линию отрезка
                qreal t = QPointF::dotProduct(pointVec, lineVec) / lineLen2;

                QPointF projection;
                if (t < 0) {
                    projection = p1; //ближайшая точка - начало отрезка
                }
                else if (t > 1) {
                    projection = p2; //ближайшая точка - конец отрезка
                }
                else {
                    projection = p1 + t * lineVec; //проекция лежит на отрезке
                }
                distance = QLineF(clickPos, projection).length();
            }
        }
        // НОВОЕ: если примитив - "Окружность"
        else if (primitive->getType() == PrimitiveType::Circle) {
            auto* circle = static_cast<CirclePrimitive*>(primitive.get());
            QPointF center(circle->getCenter().getX(), circle->getCenter().getY());

            // Расстояние от клика до центра
            double distToCenter = QLineF(clickPos, center).length();

            // Расстояние до "обода" окружности = |дистанция_до_центра - радиус|
            distance = std::abs(distToCenter - circle->getRadius());
        }

        //корректировка на порог попадания
        if (distance < minDistance) {
            minDistance = distance;
            primitiveToHit = primitive.get();
        }
    }

    //если объект найден, то посылается сигнал в MainWindow
    if (primitiveToHit) {
        emit primitiveHit(primitiveToHit);
    }
}

void DeleteTool::reset()
{
    //возвращается вид курсора
    QApplication::restoreOverrideCursor();
}

void DeleteTool::onMouseMove(QMouseEvent* event, Scene* scene, ViewportPanelWidget* viewport) { Q_UNUSED(event); Q_UNUSED(scene); Q_UNUSED(viewport); }
void DeleteTool::onMouseRelease(QMouseEvent* event, Scene* scene, ViewportPanelWidget* viewport) { Q_UNUSED(event); Q_UNUSED(scene); Q_UNUSED(viewport); }
