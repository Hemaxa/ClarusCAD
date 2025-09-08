#include "ToolbarPanelWidget.h"

#include <QToolButton>
#include <QVBoxLayout>

ToolbarPanelWidget::ToolbarPanelWidget(const QString& title, QWidget* parent)
    : BaseDockWidget(title, parent)
{
    auto* panel = new QWidget();
    auto* layout = new QVBoxLayout(panel);
    layout->setAlignment(Qt::AlignTop);

    auto* createSegmentBtn = new QToolButton();
    createSegmentBtn->setText("Отрезок");
    createSegmentBtn->setCheckable(true);
    createSegmentBtn->setChecked(true);
    layout->addWidget(createSegmentBtn);

    connect(createSegmentBtn, &QToolButton::clicked, this, &ToolbarPanelWidget::segmentToolActivated);

    setWidget(panel);
}
