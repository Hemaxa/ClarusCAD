#pragma once
#include "BaseDockWidget.h"

class QListWidget;
class Scene;

class SceneObjectsPanelWidget : public BaseDockWidget
{
    Q_OBJECT

public:
    explicit SceneObjectsPanelWidget(const QString& title, QWidget* parent = nullptr);

public slots:
    void updateView(const Scene* scene);

private:
    QListWidget* m_listWidget;
};
