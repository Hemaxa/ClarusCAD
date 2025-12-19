#include "CommonPropertiesWidget.h"
#include "BasePrimitive.h"
#include "ThemeManager.h"

#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QFormLayout>

CommonPropertiesWidget::CommonPropertiesWidget(QWidget* parent)
    : BasePropertiesWidget(parent)
{
    // Настройка кнопки
    m_applyButton->setText("Применить");
    connect(m_applyButton, &QPushButton::clicked, this, &CommonPropertiesWidget::onApplyClicked);

    // Скрываем центральную колонку (нет геометрических параметров)
    m_centralColumn->hide();
}

void CommonPropertiesWidget::updateFieldValues()
{
    // Обновляем текст кнопки
    int count = m_selectedPrimitives.size();
    m_applyButton->setText(QString("Применить ко всем (%1)").arg(count));
}

void CommonPropertiesWidget::onApplyClicked()
{
    // Отправляем сигнал с текущими выбранными настройками
    emit commonPropertiesApplied(m_selectedColor, m_selectedLineTypeId);
}
