#include "PolygonPropertiesWidget.h"
#include "PolygonPrimitive.h"
#include "ThemeManager.h"

#include <QFormLayout>
#include <QSpinBox>
#include <QComboBox>
#include <QPushButton>

PolygonPropertiesWidget::PolygonPropertiesWidget(QWidget* parent) : BasePropertiesWidget(parent)
{
    //заполнение панели декартовых координат (используем для параметров многоугольника)
    auto* cartesianLayout = static_cast<QFormLayout*>(m_cartesianWidgets->layout());
    
    //количество углов
    m_sidesSpinBox = new QSpinBox();
    m_sidesSpinBox->setRange(3, 100);
    m_sidesSpinBox->setValue(6);
    m_sidesSpinBox->setFixedWidth(80);
    cartesianLayout->addRow("Кол-во углов:", m_sidesSpinBox);
    
    //тип многоугольника
    m_typeComboBox = new QComboBox();
    m_typeComboBox->addItem("Вписанный", static_cast<int>(PolygonCreationMode::Inscribed));
    m_typeComboBox->addItem("Описанный", static_cast<int>(PolygonCreationMode::Circumscribed));
    m_typeComboBox->setFixedWidth(130);
    cartesianLayout->addRow("Тип:", m_typeComboBox);

    //подключение сигналов от контролов для обновления инструмента в реальном времени
    connect(m_sidesSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), 
            this, &PolygonPropertiesWidget::onSidesChanged);
    connect(m_typeComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), 
            this, &PolygonPropertiesWidget::onTypeChanged);

    //подключение сигнала от кнопки
    connect(m_applyButton, &QPushButton::clicked, this, &PolygonPropertiesWidget::onApplyButtonClicked);

    //установка системы координат по-умолчанию
    setCoordinateSystem(CoordinateSystemType::Cartesian);
}

void PolygonPropertiesWidget::setPrimitives(const QList<BasePrimitive*>& primitives)
{
    BasePropertiesWidget::setPrimitives(primitives);

    m_currentPolygon = nullptr;
    if (m_currentPrimitive) {
        m_currentPolygon = dynamic_cast<PolygonPrimitive*>(m_currentPrimitive);
    }

    updateFieldValues();
}

void PolygonPropertiesWidget::updateFieldValues()
{
    //если существующий объект
    if (m_currentPolygon) {
        m_sidesSpinBox->setValue(m_currentPolygon->getSides());
        
        PolygonType pType = m_currentPolygon->getPolygonType();
        int index = (pType == PolygonType::Inscribed) ? 0 : 1;
        m_typeComboBox->setCurrentIndex(index);
    }
    //если новый объект - оставляем текущие значения (последние использованные)
}

void PolygonPropertiesWidget::updatePrompt()
{
    //подсказка для многоугольника (пока оставим пустой, можно добавить svg позже)
    m_leftColumn->clear();
    m_leftColumn->setText("Многоугольник\n\n1. Укажите центр\n2. Укажите радиус");
}

void PolygonPropertiesWidget::onApplyButtonClicked()
{
    int sides = m_sidesSpinBox->value();
    PolygonCreationMode mode = static_cast<PolygonCreationMode>(
        m_typeComboBox->currentData().toInt());
    
    int typeToEmit = m_selectedLineTypeId;
    if (typeToEmit == -1 && m_currentPolygon) {
        typeToEmit = m_currentPolygon->getLineType();
    }
    if (typeToEmit == -1) typeToEmit = static_cast<int>(LineType::SolidMain);

    emit propertiesApplied(m_currentPolygon, sides, mode, 
                           m_selectedColor, static_cast<LineType>(typeToEmit));
}

void PolygonPropertiesWidget::onSidesChanged(int value)
{
    emit sidesChanged(value);
}

void PolygonPropertiesWidget::onTypeChanged(int index)
{
    PolygonCreationMode mode = static_cast<PolygonCreationMode>(
        m_typeComboBox->itemData(index).toInt());
    emit polygonTypeChanged(mode);
}
