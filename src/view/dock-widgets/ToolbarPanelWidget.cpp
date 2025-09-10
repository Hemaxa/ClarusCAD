#include "ToolbarPanelWidget.h"

#include <QToolButton>
#include <QVBoxLayout>

ToolbarPanelWidget::ToolbarPanelWidget(const QString& title, QWidget* parent)
    : BaseDockWidget(title, parent)
{
    auto* layout = new QVBoxLayout(canvas());
    layout->setAlignment(Qt::AlignTop);

    auto* createSegmentBtn = new QToolButton();
    createSegmentBtn->setText("Отрезок");
    createSegmentBtn->setCheckable(true);
    createSegmentBtn->setChecked(true);
    layout->addWidget(createSegmentBtn);

    connect(createSegmentBtn, &QToolButton::clicked, this, &ToolbarPanelWidget::segmentToolActivated);

    setMinimumWidth(200);
}
