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
    // m_leftColumn это QLabel*, используем его напрямую для иконки
    m_leftColumn->setFixedSize(64, 64);
    m_leftColumn->setAlignment(Qt::AlignCenter);

    // Настройка кнопки
    m_applyButton->setText("Применить");
    connect(m_applyButton, &QPushButton::clicked, this, &CommonPropertiesWidget::onApplyClicked);

    // Скрываем центральную колонку (нет геометрических параметров)
    m_centralColumn->hide();
}

void CommonPropertiesWidget::updateFieldValues()
{
    // Нет дополнительных полей для обновления
}

void CommonPropertiesWidget::updatePrompt()
{
    // Показываем иконку для мультивыделения разнотипных объектов
    // Используем m_leftColumn напрямую (это QLabel)
    m_leftColumn->setText("◈");
    m_leftColumn->setStyleSheet("font-size: 48px; color: #00ff7f;");

    // Обновляем текст кнопки
    int count = m_selectedPrimitives.size();
    m_applyButton->setText(QString("Применить ко всем (%1)").arg(count));
}

void CommonPropertiesWidget::onApplyClicked()
{
    // Отправляем сигнал с текущими выбранными настройками
    emit commonPropertiesApplied(m_selectedColor, m_selectedLineTypeId);
}
