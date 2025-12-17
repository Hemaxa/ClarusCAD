#include "DeleteTool.h"
#include "Scene.h"
#include "ViewportPanelWidget.h"
#include "BasePrimitive.h"
#include <QMouseEvent>
#include <QApplication>

DeleteTool::DeleteTool(QObject* parent) : BaseCreationTool(parent) {}

void DeleteTool::onMousePress(QMouseEvent* event, Scene* scene, ViewportPanelWidget* viewport)
{
    if (event->button() != Qt::LeftButton) return;

    // Вьюпорт сам переводит координаты в мировые в event,
    // но если здесь сырые - используем viewport->screenToWorld.
    // В новой реализации Viewport передает в onMousePress уже трансформированный event.
    QPointF clickPos = event->position();

    // Допуск 10 пикселей / зум
    double tolerance = 10.0 / viewport->getZoomFactor();

    BasePrimitive* primitiveToHit = nullptr;

    // Универсальный проход по всем примитивам
    // Больше никаких if(type == Segment)...
    for (const auto& primitive : scene->getPrimitives()) {
        if (primitive->hitTest(clickPos, tolerance)) {
            primitiveToHit = primitive.get();
            // Берем последний (верхний), поэтому не делаем break
        }
    }

    if (primitiveToHit) {
        emit primitiveHit(primitiveToHit);
    }
}

void DeleteTool::reset() {
    QApplication::restoreOverrideCursor();
}
