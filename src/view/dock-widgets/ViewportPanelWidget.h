#pragma once

#include "BaseDockWidget.h"

class Scene;
class BaseTool;

class ViewportPanelWidget : public BaseDockWidget
{
    Q_OBJECT

public:
    explicit ViewportPanelWidget(const QString& title, QWidget* parent = nullptr);

    void setScene(Scene* scene);
    void setActiveTool(BaseTool* tool);
    void update();

    // Мы больше не переопределяем paintEvent и события мыши здесь
    // Вместо этого мы будем обрабатывать их по-другому

private:
    // Эти поля теперь будут отвечать за данные для отрисовки на холсте
    Scene* m_scene = nullptr;
    BaseTool* m_activeTool = nullptr;

    // Метод для отрисовки, который будет вызываться событием
    void paintCanvas(QPaintEvent* event);
    bool eventFilter(QObject* obj, QEvent* event);
};
