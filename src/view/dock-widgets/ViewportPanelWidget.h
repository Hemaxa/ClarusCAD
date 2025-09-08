#pragma once

#include <QWidget>

class Scene;
class BaseTool;

class ViewportPanelWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ViewportPanelWidget(QWidget* parent = nullptr);

    void setScene(Scene* scene);
    void setActiveTool(BaseTool* tool);

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;

private:
    Scene* m_scene = nullptr;
    BaseTool* m_activeTool = nullptr;
};
