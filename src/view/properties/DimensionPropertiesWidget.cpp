// DimensionPropertiesWidget.cpp

#include "DimensionPropertiesWidget.h"
#include "LinearDimensionPrimitive.h"

#include <QFormLayout>
#include <QVBoxLayout>

DimensionPropertiesWidget::DimensionPropertiesWidget(QWidget* parent) : BasePropertiesWidget(parent)
{
    auto* layout = new QFormLayout(m_centralColumn);

    m_measuredValueLabel = new QLabel("0.00", this);
    m_customTextEdit = new QLineEdit(this);
    m_customTextEdit->setPlaceholderText("Авто (поставьте текст для переопределения)");

    layout->addRow("Измеренное значение:", m_measuredValueLabel);
    layout->addRow("Текст размера:", m_customTextEdit);

    connect(m_customTextEdit, &QLineEdit::textEdited, this, &DimensionPropertiesWidget::onCustomTextChanged);
}

void DimensionPropertiesWidget::updateFieldValues()
{
    if (!m_currentPrimitive || m_currentPrimitive->getType() != PrimitiveType::LinearDimension)
        return;

    auto* dim = static_cast<LinearDimensionPrimitive*>(m_currentPrimitive);
    m_measuredValueLabel->setText(QString::number(dim->getMeasuredValue(), 'f', 2));

    QSignalBlocker blocker(m_customTextEdit);
    m_customTextEdit->setText(dim->getCustomText());
}

void DimensionPropertiesWidget::onCustomTextChanged(const QString& text)
{
    if (!m_selectedPrimitives.isEmpty()) {
        for (auto* prim : m_selectedPrimitives) {
            if (prim->getType() == PrimitiveType::LinearDimension) {
                static_cast<LinearDimensionPrimitive*>(prim)->setCustomText(text);
            }
        }
        emit dimensionPropertiesApplied();
    }
}
