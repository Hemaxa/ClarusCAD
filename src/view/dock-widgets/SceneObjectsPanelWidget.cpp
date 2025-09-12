#include "SceneObjectsPanelWidget.h"
#include "Scene.h"
#include "BasePrimitive.h"

#include <QListWidget>
#include <QVBoxLayout>

SceneObjectsPanelWidget::SceneObjectsPanelWidget(const QString& title, QWidget* parent) : BaseDockWidget(title, parent)
{
    //создание экземпляра QListWidget
    m_listWidget = new QListWidget();

    auto* layout = new QVBoxLayout(canvas()); //вертикальный шаблон компоновки
    layout->setContentsMargins(0, 0, 0, 0); //убирает отступы, чтобы занять всю допустимую область панели
    layout->addWidget(m_listWidget); //добавление содержимого панели в canvas

    //сигнал о смене выбранного объекта
    connect(m_listWidget, &QListWidget::currentItemChanged, this, &SceneObjectsPanelWidget::onSelectionChanged);

    //минимальная ширина окна
    setMinimumWidth(200);
}

void SceneObjectsPanelWidget::updateView(const Scene* scene)
{
    //если нет указателя на сцену, функция не отрабатывает
    if (!scene) return;

    //сохранение указателя на сцену
    m_currentScene = scene;

    //удаление старых объектов списка
    m_listWidget->clear();

    //получение из класса Scene ссылку на все объекты сцены
    const auto& primitives = scene->getPrimitives();
    for (int i = 0; i < primitives.size(); ++i) {
        //цикл записи примитивов "Отрезок"
        if (primitives[i]->getType() == PrimitiveType::Segment) {
            m_listWidget->addItem(QString("Отрезок %1").arg(i + 1));
        }
    }
}

void SceneObjectsPanelWidget::onSelectionChanged()
{
    //если нет указателя на сохраненную сцену, функция не отрабатывает
    if (!m_currentScene) return;

    int index = m_listWidget->currentRow(); //получение индекса выбранной строки
    const auto& primitives = m_currentScene->getPrimitives();

    //проверка, что индекс корректен
    if (index >= 0 && index < primitives.size()) {
        //получение указателя на примитив из сцены по этому индексу
        BasePrimitive* selectedPrimitive = primitives[index].get();

        //отправка сигнала, что примитив выбран
        emit primitiveSelected(selectedPrimitive);
    }
}
