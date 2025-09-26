#include "BasePropertiesWidget.h"
#include "BasePrimitive.h"

#include <QPushButton>
#include <QColorDialog>
#include <QFormLayout>
#include <QStackedWidget>
#include <QVBoxLayout>

BasePropertiesWidget::BasePropertiesWidget(QWidget* parent) : QWidget(parent)
{
    //тип системы координат по-умолчанию
    m_coordSystem = CoordinateSystemType::Cartesian;

    //кнопка выбора цвета
    m_colorButton = new QPushButton(this);
    m_colorButton->setFixedSize(24, 24);
    m_colorButton->setCursor(Qt::PointingHandCursor);
    connect(m_colorButton, &QPushButton::clicked, this, &BasePropertiesWidget::onColorButtonClicked);

    //кнопка "Создать" / "Обновить"
    m_applyButton = new QPushButton(this);

    //создание и добавление виджетов систем координат
    m_stack = new QStackedWidget(this);
    m_cartesianWidgets = new QWidget(this);
    m_polarWidgets = new QWidget(this);
    m_basePref = new QWidget(this);

    new QFormLayout(m_cartesianWidgets);
    new QFormLayout(m_polarWidgets);

    m_stack->addWidget(m_cartesianWidgets);
    m_stack->addWidget(m_polarWidgets);
}

void BasePropertiesWidget::setPrimitive(BasePrimitive* primitive)
{
    m_currentPrimitive = primitive;

    //режим редактирования
    if (m_currentPrimitive) {
        m_selectedColor = m_currentPrimitive->getColor();
        m_applyButton->setText("Обновить");
    }
    //режим создания
    else
    {
        m_applyButton->setText("Создать");
    }

    m_colorButton->setStyleSheet(QString("background-color: %1; border-radius: 12px; border: 1px solid gray;").arg(m_selectedColor.name()));
    updateFieldValues();
}

void BasePropertiesWidget::setCoordinateSystem(CoordinateSystemType type)
{
    m_coordSystem = type;
    updateFieldsVisibility();
    updateFieldValues();
}

void BasePropertiesWidget::updateColor(const QColor& color)
{
    m_selectedColor = color;
    m_colorButton->setStyleSheet(QString("background-color: %1; border-radius: 12px; border: 1px solid gray;").arg(m_selectedColor.name()));
}

void BasePropertiesWidget::onColorButtonClicked()
{
    QColor color = QColorDialog::getColor(m_selectedColor, this, "Выберите цвет");
    if (color.isValid()) {
        m_selectedColor = color;
        m_colorButton->setStyleSheet(QString("background-color: %1; border-radius: 12px; border: 1px solid gray;").arg(m_selectedColor.name()));
        emit colorChanged(m_selectedColor);
    }
}

void BasePropertiesWidget::updateFieldsVisibility()
{
    if (m_coordSystem == CoordinateSystemType::Cartesian) {
        m_stack->setCurrentWidget(m_cartesianWidgets);
    }
    else {
        m_stack->setCurrentWidget(m_polarWidgets);
    }
}
