//SceneObjectsPanelWidget - панель объектов сцены

#pragma once

#include "BasePanelWidget.h"

class QListWidget;
class Scene;
class BasePrimitive;

//наслдедуется от базового класса BasePanelWidget
class SceneObjectsPanelWidget : public BasePanelWidget
{
    Q_OBJECT

public:
    //конструктор
    explicit SceneObjectsPanelWidget(const QString& title, QWidget* parent = nullptr);

public slots:
    //слот обновления списка объектов (подключен к сигналу sceneChanged из MainWindow)
    void update(const Scene* scene);

signals:
    //сигнал, информирующий о выборе объектов в списке
    void primitivesSelected(const QList<BasePrimitive*>& primitives);

private slots:
    //слот, реагирующий на изменение выбора в списке
    void onSelectionChanged();

private:
    QListWidget* m_listWidget; //список объектов в сцене
    const Scene* m_currentScene = nullptr; //указатель на текущую сцену
};
