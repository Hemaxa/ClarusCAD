//DeleteTool - инструмент удаления объектов со сцены

#pragma once
#include "BaseCreationTool.h"

class DeleteTool : public BaseCreationTool
{
    Q_OBJECT
public:
    /**
     * @brief Конструктор инструмента удаления.
     */
    explicit DeleteTool(QObject* parent = nullptr);

    void onMousePress(QMouseEvent* event, Scene* scene, ViewportPanelWidget* viewport) override;
    void onMouseMove(QMouseEvent*, Scene*, ViewportPanelWidget*) override {}; //< Не используется
    void onMouseRelease(QMouseEvent*, Scene*, ViewportPanelWidget*) override {}; //< Не используется
    void reset() override;

signals:
    /**
     * @brief Сигнал, что примитив был удален (или помечен на удаление).
     * @param primitive Указатель на примитив.
     */
    void primitiveHit(BasePrimitive* primitive);
};
