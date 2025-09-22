#include "BasePropertiesWidget.h"
#include "BasePrimitive.h"

#include <QPushButton>
#include <QColorDialog>
#include <QFormLayout>

BasePropertiesWidget::BasePropertiesWidget(QWidget* parent) : QWidget(parent)
{
    m_layout = new QFormLayout(this);

    //кнопка выбора цвета
    m_colorButton = new QPushButton(this);
    m_colorButton->setFixedSize(24, 24);
    m_colorButton->setCursor(Qt::PointingHandCursor);
    connect(m_colorButton, &QPushButton::clicked, this, &BasePropertiesWidget::onColorButtonClicked);

    //кнопка "Создать" / "Обновить"
    m_applyButton = new QPushButton(this);
}

void BasePropertiesWidget::setPrimitive(BasePrimitive* primitive)
{
    m_currentPrimitive = primitive;

    //режим редактирования
    if (m_currentPrimitive) {
        // РЕЖИМ РЕДАКТИРОВАНИЯ
        m_selectedColor = m_currentPrimitive->getColor();
        m_applyButton->setText("Обновить");
    }
    //режим создания
    else
    {
        m_selectedColor = Qt::white;
        m_applyButton->setText("Создать");
    }

    //обновление цвета кружка-кнопки
    m_colorButton->setStyleSheet(QString("background-color: %1; border-radius: 12px; border: 1px solid gray;").arg(m_selectedColor.name()));
}

void BasePropertiesWidget::onColorButtonClicked()
{
    QColor color = QColorDialog::getColor(m_selectedColor, this, "Выберите цвет");
    if (color.isValid()) {
        m_selectedColor = color;
        m_colorButton->setStyleSheet(QString("background-color: %1; border-radius: 12px; border: 1px solid gray;").arg(m_selectedColor.name()));
    }
}
