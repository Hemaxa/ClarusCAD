#include "SplinePropertiesWidget.h"
#include "SplinePrimitive.h"
#include "ThemeManager.h"

#include <QFormLayout>
#include <QCheckBox>
#include <QPushButton>

SplinePropertiesWidget::SplinePropertiesWidget(QWidget* parent) : BasePropertiesWidget(parent)
{
    //заполнение панели декартовых координат (используем для параметров сплайна)
    auto* cartesianLayout = static_cast<QFormLayout*>(m_cartesianWidgets->layout());
    
    //замкнутый/разомкнутый
    m_closedCheckBox = new QCheckBox("Замкнутый сплайн");
    m_closedCheckBox->setChecked(false);
    cartesianLayout->addRow("", m_closedCheckBox);

    //подключение сигнала для обновления инструмента в реальном времени
    connect(m_closedCheckBox, &QCheckBox::toggled, 
            this, &SplinePropertiesWidget::onClosedChanged);

    //подключение сигнала от кнопки
    connect(m_applyButton, &QPushButton::clicked, this, &SplinePropertiesWidget::onApplyButtonClicked);

    //установка системы координат по-умолчанию
    setCoordinateSystem(CoordinateSystemType::Cartesian);
}

void SplinePropertiesWidget::setPrimitives(const QList<BasePrimitive*>& primitives)
{
    BasePropertiesWidget::setPrimitives(primitives);

    m_currentSpline = nullptr;
    if (m_currentPrimitive) {
        m_currentSpline = dynamic_cast<SplinePrimitive*>(m_currentPrimitive);
    }

    updateFieldValues();
}

void SplinePropertiesWidget::updateFieldValues()
{
    //если существующий объект
    if (m_currentSpline) {
        m_closedCheckBox->setChecked(m_currentSpline->isClosed());
    }
    //если новый объект - оставляем текущие значения
}

void SplinePropertiesWidget::updatePrompt()
{
    //подсказка для сплайна
    m_leftColumn->clear();
    m_leftColumn->setText("Сплайн\n\n1. Указывайте\n   контрольные точки\n2. ПКМ или Enter\n   для завершения\n3. Backspace для\n   отмены точки");
}

void SplinePropertiesWidget::onApplyButtonClicked()
{
    bool closed = m_closedCheckBox->isChecked();
    
    int typeToEmit = m_selectedLineTypeId;
    if (typeToEmit == -1 && m_currentSpline) {
        typeToEmit = m_currentSpline->getLineType();
    }
    if (typeToEmit == -1) typeToEmit = static_cast<int>(LineType::SolidMain);

    emit propertiesApplied(m_currentSpline, closed, 
                           m_selectedColor, static_cast<LineType>(typeToEmit));
}

void SplinePropertiesWidget::onClosedChanged(bool checked)
{
    emit closedChanged(checked);
}
