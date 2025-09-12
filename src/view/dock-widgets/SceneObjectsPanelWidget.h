//SceneObjectsPanelWidget - панель объектов сцены

#pragma once

#include "BaseDockWidget.h"

class QListWidget;
class Scene;
class BasePrimitive;

//наслдедуется от базового класса BaseDockWidget
class SceneObjectsPanelWidget : public BaseDockWidget
{
    Q_OBJECT

public:
    //конструктор
    explicit SceneObjectsPanelWidget(const QString& title, QWidget* parent = nullptr);

public slots:
    //слот обновления списка объектов
    void updateView(const Scene* scene);

signals:
    //сигнал, информирующий о выборе объекта в списке
    void primitiveSelected(BasePrimitive* primitive);

private slots:
    //слот, реагирующий на изменение выбора в списке
    void onSelectionChanged();

private:
    QListWidget* m_listWidget; //список объектов в сцене
    const Scene* m_currentScene = nullptr; //указатель на текущую сцену
};
