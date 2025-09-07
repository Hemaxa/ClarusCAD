#include "ToolbarPanel.h"

#include <QToolButton>
#include <QVBoxLayout>
#include <QIcon>
#include <QButtonGroup>

ToolbarPanel::ToolbarPanel(QWidget* parent) : QWidget(parent)
{
    auto* layout = new QVBoxLayout(this);
    layout->setAlignment(Qt::AlignTop);

    m_toolGroup = new QButtonGroup(this);
    m_toolGroup->setExclusive(true); // Только одна кнопка может быть активна

    auto* createLineButton = new QToolButton();
    createLineButton->setText("Отрезок");
    // createLineButton->setIcon(QIcon(":/icons/segment_tool.svg"));
    createLineButton->setCheckable(true);
    createLineButton->setChecked(true); // Активен по умолчанию

    m_toolGroup->addButton(createLineButton);
    layout->addWidget(createLineButton);

    connect(createLineButton, &QToolButton::clicked, this, &ToolbarPanel::createLineToolActivated);
}
