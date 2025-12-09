#include "RectanglePropertiesWidget.h"
#include "RectanglePrimitive.h"
#include <QFormLayout>
#include <QDoubleValidator>
#include <QPushButton>

RectanglePropertiesWidget::RectanglePropertiesWidget(QWidget* parent) : BasePropertiesWidget(parent)
{
    auto* validator = new QDoubleValidator(this);

    auto* layout = static_cast<QFormLayout*>(m_cartesianWidgets->layout());

    m_centerX = new QLineEdit("0.0"); m_centerX->setValidator(validator);
    m_centerY = new QLineEdit("0.0"); m_centerY->setValidator(validator);
    m_width   = new QLineEdit("0.0"); m_width->setValidator(validator);
    m_height  = new QLineEdit("0.0"); m_height->setValidator(validator);
    m_rotation= new QLineEdit("0.0"); m_rotation->setValidator(validator);

    layout->addRow("Центр X:", m_centerX);
    layout->addRow("Центр Y:", m_centerY);
    layout->addRow("Ширина:", m_width);
    layout->addRow("Высота:", m_height);
    layout->addRow("Поворот (°):", m_rotation);

    connect(m_applyButton, &QPushButton::clicked, this, &RectanglePropertiesWidget::onApplyButtonClicked);
}

void RectanglePropertiesWidget::setPrimitives(const QList<BasePrimitive*>& primitives)
{
    BasePropertiesWidget::setPrimitives(primitives);
    m_currentRect = nullptr;
    if(m_currentPrimitive && m_currentPrimitive->getType() == PrimitiveType::Rectangle) {
        m_currentRect = static_cast<RectanglePrimitive*>(m_currentPrimitive);
    }
    updateFieldValues();
}

void RectanglePropertiesWidget::updateFieldValues()
{
    if(m_currentRect) {
        m_centerX->setText(QString::number(m_currentRect->getCenter().getX()));
        m_centerY->setText(QString::number(m_currentRect->getCenter().getY()));
        m_width->setText(QString::number(m_currentRect->getWidth()));
        m_height->setText(QString::number(m_currentRect->getHeight()));
        m_rotation->setText(QString::number(m_currentRect->getRotation()));
    }
}

void RectanglePropertiesWidget::onApplyButtonClicked()
{
    double cx = m_centerX->text().toDouble();
    double cy = m_centerY->text().toDouble();
    double w = m_width->text().toDouble();
    double h = m_height->text().toDouble();
    double r = m_rotation->text().toDouble();

    int typeEmit = m_selectedLineTypeId;
    if(typeEmit == -1 && m_currentRect) typeEmit = m_currentRect->getLineType();
    if(typeEmit == -1) typeEmit = (int)LineType::SolidMain;

    emit propertiesApplied(m_currentRect, PointPrimitive(cx, cy), w, h, r, m_selectedColor, (LineType)typeEmit);
}
