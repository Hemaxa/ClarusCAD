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
    /**
     * @brief Конструктор панели списка объектов.
     */
    explicit SceneObjectsPanelWidget(const QString& title, QWidget* parent = nullptr);

public slots:
    /**
     * @brief Обновить список объектов в панели.
     * @param scene Указатель на сцену с актуальными объектами.
     */
    void update(const Scene* scene);

signals:
    /**
     * @brief Сигнал о выборе объектов в списке.
     * @param primitives Список выбранных примитивов.
     */
    void primitivesSelected(const QList<BasePrimitive*>& primitives);

private slots:
    /**
     * @brief Слот обработки изменения выделения в QListWidget.
     */
    void onSelectionChanged();

private:
    QListWidget* m_listWidget;        ///< Виджет списка
    const Scene* m_currentScene = nullptr; ///< Ссылка на текущую сцену
};
