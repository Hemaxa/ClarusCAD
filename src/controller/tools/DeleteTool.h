//DeleteTool - инструмент удаления объектов

#pragma once

#include "BaseCreationTool.h"

class DeleteTool : public BaseCreationTool
{
    Q_OBJECT

public:
    //конструктор
    explicit DeleteTool(QObject* parent = nullptr);

    //переопределение методов действий мыши
    void onMousePress(QMouseEvent* event, Scene* scene, ViewportPanelWidget* viewport) override;
    void onMouseMove(QMouseEvent* event, Scene* scene, ViewportPanelWidget* viewport) override;
    void onMouseRelease(QMouseEvent* event, Scene* scene, ViewportPanelWidget* viewport) override;

    //переопределение метода очистки инструмента
    void reset() override;

signals:
    //сигнал, информирующий MainWindow об удалении объекта
    void primitiveHit(BasePrimitive* primitive);
};
