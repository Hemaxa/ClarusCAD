#include "DeleteTool.h"
#include "Scene.h"
#include "ViewportPanelWidget.h"
#include "SegmentPrimitive.h"

#include <QApplication>
#include <QCursor>
#include <QMouseEvent>

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

            //проекция точки на линию отрезка
            qreal t = QPointF::dotProduct(pointVec, lineVec) / QPointF::dotProduct(lineVec, lineVec);

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

            double distance = QLineF(clickPos, projection).length();

            //корректировка на порог попадания
            if (distance < minDistance) {
                minDistance = distance;
                primitiveToHit = primitive.get();
            }
        }
    }

    //если объект найден, то посылается сигнал в MainWindow
    if (primitiveToHit) {
        emit primitiveHit(primitiveToHit);
    }
}

void DeleteTool::onMouseMove(QMouseEvent* event, Scene* scene, ViewportPanelWidget* viewport) {}
void DeleteTool::onMouseRelease(QMouseEvent* event, Scene* scene, ViewportPanelWidget* viewport) {}

void DeleteTool::reset()
{
    //возвращается вид курсора
    QApplication::restoreOverrideCursor();
}
