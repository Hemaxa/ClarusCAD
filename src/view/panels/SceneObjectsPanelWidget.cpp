#include "SceneObjectsPanelWidget.h"
#include "Scene.h"
#include "BasePrimitive.h"

#include <QListWidget>
#include <QVBoxLayout>

SceneObjectsPanelWidget::SceneObjectsPanelWidget(const QString& title, QWidget* parent) : BasePanelWidget(title, parent)
{
    //создание экземпляра QListWidget для списка
    m_listWidget = new QListWidget();

    //включение множественного выделения
    m_listWidget->setSelectionMode(QAbstractItemView::ExtendedSelection);

    auto* layout = new QVBoxLayout(canvas()); //вертикальный шаблон компоновки
    layout->setContentsMargins(0, 0, 0, 0); //убирает отступы, чтобы занять всю допустимую область панели
    layout->addWidget(m_listWidget); //добавление содержимого панели в canvas

    //сигнал о смене выбранного объекта
    connect(m_listWidget, &QListWidget::currentItemChanged, this, &SceneObjectsPanelWidget::onSelectionChanged);

    //минимальная ширина окна
    setMinimumWidth(200);
}

void SceneObjectsPanelWidget::update(const Scene* scene)
{
    //если нет указателя на обновленную сцену, функция не отрабатывает
    if (!scene) return;

    //сохранение указателя на сцену
    m_currentScene = scene;

    // 1. ЗАПОМИНАЕМ ТЕКУЩИЕ ВЫДЕЛЕННЫЕ ИМЕНА
    QList<QString> selectedNames;
    for (auto* item : m_listWidget->selectedItems()) {
        selectedNames.append(item->text());
    }

    m_listWidget->blockSignals(true);
    m_listWidget->clear();

    const auto& primitives = scene->getPrimitives();
    for (int i = 0; i < primitives.size(); ++i) {
        m_listWidget->addItem(primitives[i]->getName());
    }

    // 2. ВОССТАНАВЛИВАЕМ ВЫДЕЛЕНИЕ
    for (int i = 0; i < m_listWidget->count(); ++i) {
        QListWidgetItem* item = m_listWidget->item(i);
        if (selectedNames.contains(item->text())) {
            item->setSelected(true);
        }
    }

    m_listWidget->blockSignals(false);
}

void SceneObjectsPanelWidget::onSelectionChanged()
{
    if (!m_currentScene) return;

    // Собираем список всех выбранных примитивов
    QList<BasePrimitive*> selectedPrimitives;
    const auto& primitives = m_currentScene->getPrimitives();

    for (int i = 0; i < m_listWidget->count(); ++i) {
        if (m_listWidget->item(i)->isSelected()) {
            if (i >= 0 && i < primitives.size()) {
                selectedPrimitives.append(primitives[i].get());
            }
        }
    }

    // Отправляем список (сигнатуру сигнала нужно изменить в .h)
    emit primitivesSelected(selectedPrimitives);
}
