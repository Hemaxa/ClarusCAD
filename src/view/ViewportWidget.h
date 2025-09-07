#pragma once

#include <QWidget>

class Scene;

class ViewportWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ViewportWidget(QWidget *parent = nullptr);
    void setScene(Scene* scene);

private:
    Scene* m_scene = nullptr;

protected:
    void paintEvent(QPaintEvent *event) override;
};
