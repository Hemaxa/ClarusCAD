#include "ArcPropertiesWidget.h"
#include "ArcPrimitive.h"
#include <QFormLayout>
#include <QDoubleValidator>
#include <QPushButton>

ArcPropertiesWidget::ArcPropertiesWidget(QWidget* parent) : BasePropertiesWidget(parent)
{
    auto* validator = new QDoubleValidator(this);
    auto* layout = static_cast<QFormLayout*>(m_cartesianWidgets->layout());

    m_centerX = new QLineEdit("0.0"); m_centerX->setValidator(validator);
    m_centerY = new QLineEdit("0.0"); m_centerY->setValidator(validator);
    m_radius  = new QLineEdit("0.0"); m_radius->setValidator(validator);
    m_startAngle = new QLineEdit("0.0"); m_startAngle->setValidator(validator);
    m_spanAngle  = new QLineEdit("0.0"); m_spanAngle->setValidator(validator);

    layout->addRow("Центр X:", m_centerX);
    layout->addRow("Центр Y:", m_centerY);
    layout->addRow("Радиус:", m_radius);
    layout->addRow("Нач. угол:", m_startAngle);
    layout->addRow("Сектор:", m_spanAngle);

    connect(m_applyButton, &QPushButton::clicked, this, &ArcPropertiesWidget::onApplyButtonClicked);
}

void ArcPropertiesWidget::setPrimitives(const QList<BasePrimitive*>& primitives)
{
    BasePropertiesWidget::setPrimitives(primitives);
    m_currentArc = nullptr;
    if(m_currentPrimitive && m_currentPrimitive->getType() == PrimitiveType::Arc) {
        m_currentArc = static_cast<ArcPrimitive*>(m_currentPrimitive);
    }
    updateFieldValues();
}

void ArcPropertiesWidget::updateFieldValues()
{
    if(m_currentArc) {
        m_centerX->setText(QString::number(m_currentArc->getCenter().getX()));
        m_centerY->setText(QString::number(m_currentArc->getCenter().getY()));
        m_radius->setText(QString::number(m_currentArc->getRadius()));
        m_startAngle->setText(QString::number(m_currentArc->getStartAngle()));
        m_spanAngle->setText(QString::number(m_currentArc->getSpanAngle()));
    }
}

void ArcPropertiesWidget::onApplyButtonClicked()
{
    double cx = m_centerX->text().toDouble();
    double cy = m_centerY->text().toDouble();
    double r = m_radius->text().toDouble();
    double start = m_startAngle->text().toDouble();
    double span = m_spanAngle->text().toDouble();

    int typeEmit = m_selectedLineTypeId;
    if(typeEmit == -1 && m_currentArc) typeEmit = m_currentArc->getLineType();
    if(typeEmit == -1) typeEmit = (int)LineType::SolidMain;

    emit propertiesApplied(m_currentArc, PointPrimitive(cx, cy), r, start, span, m_selectedColor, (LineType)typeEmit);
}
