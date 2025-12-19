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
    connect(m_listWidget, &QListWidget::itemSelectionChanged, this, &SceneObjectsPanelWidget::onSelectionChanged);

    //минимальная ширина окна
    setMinimumWidth(160);
}

void SceneObjectsPanelWidget::update(const Scene* scene)
{
    //если нет указателя на обновленную сцену, функция не отрабатывает
    if (!scene) return;

    //сохранение указателя на сцену
    m_currentScene = scene;

    // 1. ЗАПОМИНАЕМ ТЕКУЩИЕ ВЫДЕЛЕННЫЕ ОБЪЕКТЫ (ПО УКАЗАТЕЛЯМ, А НЕ ИМЕНАМ)
    QList<quintptr> selectedPtrs;
    for (auto* item : m_listWidget->selectedItems()) {
        selectedPtrs.append(item->data(Qt::UserRole).value<quintptr>());
    }

    m_listWidget->blockSignals(true);
    m_listWidget->clear();

    const auto& primitives = scene->getPrimitives();
    for (const auto& prim : primitives) {
        auto* item = new QListWidgetItem(prim->getName());

        // Храним указатель на объект внутри элемента списка
        // reinterpret_cast нужен для преобразования указателя в число
        item->setData(Qt::UserRole, QVariant::fromValue(reinterpret_cast<quintptr>(prim.get())));

        m_listWidget->addItem(item);
    }

    // 2. ВОССТАНАВЛИВАЕМ ВЫДЕЛЕНИЕ (Сравниваем указатели)
    for (int i = 0; i < m_listWidget->count(); ++i) {
        QListWidgetItem* item = m_listWidget->item(i);
        quintptr itemPtr = item->data(Qt::UserRole).value<quintptr>();

        if (selectedPtrs.contains(itemPtr)) {
            item->setSelected(true);
        }
    }

    m_listWidget->blockSignals(false);
}

void SceneObjectsPanelWidget::onSelectionChanged()
{
    if (!m_currentScene) return;

    QList<BasePrimitive*> selectedPrimitives;

    // Проходим по выбранным элементам UI и достаем из них указатели на реальные объекты
    for (auto* item : m_listWidget->selectedItems()) {
        quintptr ptrVal = item->data(Qt::UserRole).value<quintptr>();
        BasePrimitive* prim = reinterpret_cast<BasePrimitive*>(ptrVal);
        if (prim) {
            selectedPrimitives.append(prim);
        }
    }

    emit primitivesSelected(selectedPrimitives);
}
