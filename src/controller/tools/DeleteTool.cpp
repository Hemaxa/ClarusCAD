#include "DeleteTool.h"
#include "Scene.h"
#include "ViewportPanelWidget.h"
#include <QApplication>
#include <QCursor>

DeleteTool::DeleteTool(QObject* parent) : BaseCreationTool(parent) {}

void DeleteTool::onMousePress(QMouseEvent* event, Scene* scene, ViewportPanelWidget* viewport)
{
    // Пока не реализуем удаление по клику, будет реализовано в будущем
}

void DeleteTool::onMouseMove(QMouseEvent* event, Scene* scene, ViewportPanelWidget* viewport)
{
    // При наведении курсор меняется на крестик
    QApplication::setOverrideCursor(Qt::CrossCursor);
}

void DeleteTool::onMouseRelease(QMouseEvent* event, Scene* scene, ViewportPanelWidget* viewport)
{
    // Пустая реализация
}

void DeleteTool::reset()
{
    // Восстанавливаем курсор по умолчанию для всего приложения
    QApplication::restoreOverrideCursor();
}
