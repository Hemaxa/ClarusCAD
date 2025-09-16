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
    // Работаем только по левому клику
    if (event->button() != Qt::LeftButton) {
        return;
    }

    // Получаем мировые координаты клика
    QPointF clickPos = event->position();
    BasePrimitive* primitiveToHit = nullptr;
    double minDistance = 10.0 / viewport->getZoomFactor();  // Порог попадания (10 пикселей, скорректированные на зум)

    // Ищем ближайший к клику отрезок
    for (const auto& primitive : scene->getPrimitives()) {
        if (primitive->getType() == PrimitiveType::Segment) {
            auto* segment = static_cast<SegmentPrimitive*>(primitive.get());
            QPointF p1(segment->getStart().getX(), segment->getStart().getY());
            QPointF p2(segment->getEnd().getX(), segment->getEnd().getY());
            QLineF line(p1, p2);

            // Вектор отрезка
            QPointF lineVec = p2 - p1;
            // Вектор от начала отрезка до точки клика
            QPointF pointVec = clickPos - p1;

            // Проекция точки на линию отрезка
            qreal t = QPointF::dotProduct(pointVec, lineVec) / QPointF::dotProduct(lineVec, lineVec);

            QPointF projection;
            if (t < 0) {
                projection = p1; // Ближайшая точка - начало отрезка
            } else if (t > 1) {
                projection = p2; // Ближайшая точка - конец отрезка
            } else {
                projection = p1 + t * lineVec; // Проекция лежит на отрезке
            }

            double distance = QLineF(clickPos, projection).length();

            if (distance < minDistance) {
                minDistance = distance;
                primitiveToHit = primitive.get();
            }
        }
    }

    // Если объект найден, удаляем его
    if (primitiveToHit) {
        // >>> ИЗМЕНЕНИЕ: Вместо удаления, посылаем сигнал в MainWindow <<<
        emit primitiveHit(primitiveToHit);
    }
}

void DeleteTool::onMouseMove(QMouseEvent* event, Scene* scene, ViewportPanelWidget* viewport)
{

}

void DeleteTool::onMouseRelease(QMouseEvent* event, Scene* scene, ViewportPanelWidget* viewport)
{

}

void DeleteTool::reset()
{
    // Восстанавливаем курсор по умолчанию для всего приложения
    QApplication::restoreOverrideCursor();
}
