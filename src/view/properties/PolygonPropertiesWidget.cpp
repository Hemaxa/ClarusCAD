#include "PolygonPropertiesWidget.h"
#include "PolygonPrimitive.h"
#include "ThemeManager.h"

#include <QGridLayout>
#include <QSpinBox>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>

PolygonPropertiesWidget::PolygonPropertiesWidget(QWidget* parent) : BasePropertiesWidget(parent)
{
    auto* layout = static_cast<QGridLayout*>(m_cartesianWidgets->layout());
    
    //количество углов
    m_sidesSpinBox = new QSpinBox();
    m_sidesSpinBox->setRange(3, 100);
    m_sidesSpinBox->setValue(6);
    m_sidesSpinBox->setFixedWidth(70);
    m_sidesSpinBox->setObjectName("PropertiesInput");
    
    //тип многоугольника
    m_typeComboBox = new QComboBox();
    m_typeComboBox->addItem("Вписанный", static_cast<int>(PolygonCreationMode::Inscribed));
    m_typeComboBox->addItem("Описанный", static_cast<int>(PolygonCreationMode::Circumscribed));
    m_typeComboBox->setObjectName("PropertiesComboBox");

    //компактный grid
    layout->addWidget(new QLabel("Углы:"), 0, 0);
    layout->addWidget(m_sidesSpinBox, 0, 1);
    layout->addWidget(new QLabel("Тип:"), 0, 2);
    layout->addWidget(m_typeComboBox, 0, 3);

    //подключение сигналов
    connect(m_sidesSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), 
            this, &PolygonPropertiesWidget::onSidesChanged);
    connect(m_typeComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), 
            this, &PolygonPropertiesWidget::onTypeChanged);
    connect(m_applyButton, &QPushButton::clicked, this, &PolygonPropertiesWidget::onApplyButtonClicked);

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
    if (m_currentPolygon) {
        m_sidesSpinBox->setValue(m_currentPolygon->getSides());
        
        PolygonType pType = m_currentPolygon->getPolygonType();
        int index = (pType == PolygonType::Inscribed) ? 0 : 1;
        m_typeComboBox->setCurrentIndex(index);
    }
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
