#pragma once
#include "BaseDockWidget.h"

class QListWidget;
class Scene;
class BasePrimitive;

class SceneObjectsPanelWidget : public BaseDockWidget
{
    Q_OBJECT

public:
    explicit SceneObjectsPanelWidget(const QString& title, QWidget* parent = nullptr);

public slots:
    void updateView(const Scene* scene);

signals:
    // Сигнал, который отправляется, когда пользователь выбирает примитив в списке
    void primitiveSelected(BasePrimitive* primitive);

private slots:
    // Слот, который будет реагировать на изменение выбора в списке
    void onSelectionChanged();

private:
    QListWidget* m_listWidget;
    const Scene* m_currentScene = nullptr;
};
