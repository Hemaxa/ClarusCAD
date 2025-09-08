#include "SceneObjectsPanelWidget.h"
#include "Scene.h"

#include <QListWidget>

SceneObjectsPanelWidget::SceneObjectsPanelWidget(const QString& title, QWidget* parent)
    : BaseDockWidget(title, parent)
{
    m_listWidget = new QListWidget();
    setWidget(m_listWidget);
}

void SceneObjectsPanelWidget::updateView(const Scene* scene)
{
    if (!scene) return;

    m_listWidget->clear();

    for (const auto& primitive : scene->getPrimitives()) {
        if (primitive->getType() == PrimitiveType::Segment) {
            m_listWidget->addItem("Отрезок");
        }
    }
}
