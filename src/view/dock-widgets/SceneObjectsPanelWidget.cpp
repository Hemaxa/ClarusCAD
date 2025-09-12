#include "SceneObjectsPanelWidget.h"
#include "Scene.h"
#include "BasePrimitive.h"

#include <QListWidget>
#include <QVBoxLayout>

SceneObjectsPanelWidget::SceneObjectsPanelWidget(const QString& title, QWidget* parent) : BaseDockWidget(title, parent)
{
    m_listWidget = new QListWidget();

    auto* layout = new QVBoxLayout(canvas());
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(m_listWidget);

    connect(m_listWidget, &QListWidget::currentItemChanged, this, &SceneObjectsPanelWidget::onSelectionChanged);

    setMinimumWidth(200);
}

void SceneObjectsPanelWidget::updateView(const Scene* scene)
{
    if (!scene) return;

    m_currentScene = scene;

    m_listWidget->clear();

    m_listWidget->clear();
    const auto& primitives = scene->getPrimitives();
    for (int i = 0; i < primitives.size(); ++i) {
        if (primitives[i]->getType() == PrimitiveType::Segment) {
            m_listWidget->addItem(QString("Отрезок %1").arg(i + 1));
        }
    }
}

void SceneObjectsPanelWidget::onSelectionChanged()
{
    if (!m_currentScene) return;

    int index = m_listWidget->currentRow(); // Получаем индекс выбранной строки
    const auto& primitives = m_currentScene->getPrimitives();

    // Проверяем, что индекс корректен
    if (index >= 0 && index < primitives.size()) {
        // Получаем указатель на примитив из сцены по этому индексу
        BasePrimitive* selectedPrimitive = primitives[index].get();

        // Отправляем сигнал, что примитив выбран!
        emit primitiveSelected(selectedPrimitive);
    }
}
