#include "ArcPropertiesWidget.h"
#include "ArcPrimitive.h"
#include <QGridLayout>
#include <QDoubleValidator>
#include <QPushButton>
#include <QLabel>

ArcPropertiesWidget::ArcPropertiesWidget(QWidget* parent) : BasePropertiesWidget(parent)
{
    auto* validator = new QDoubleValidator(this);
    auto* layout = static_cast<QGridLayout*>(m_cartesianWidgets->layout());

    m_centerX = new QLineEdit("0.0"); m_centerX->setValidator(validator); m_centerX->setObjectName("PropertiesInput");
    m_centerY = new QLineEdit("0.0"); m_centerY->setValidator(validator); m_centerY->setObjectName("PropertiesInput");
    m_radius  = new QLineEdit("0.0"); m_radius->setValidator(validator); m_radius->setObjectName("PropertiesInput");
    m_startAngle = new QLineEdit("0.0"); m_startAngle->setValidator(validator); m_startAngle->setObjectName("PropertiesInput");
    m_spanAngle  = new QLineEdit("0.0"); m_spanAngle->setValidator(validator); m_spanAngle->setObjectName("PropertiesInput");

    //компактный grid: 1 ряд по 4 элемента (4 пары)
    layout->addWidget(new QLabel("X:"), 0, 0);
    layout->addWidget(m_centerX, 0, 1);
    layout->addWidget(new QLabel("Y:"), 0, 2);
    layout->addWidget(m_centerY, 0, 3);
    layout->addWidget(new QLabel("R:"), 0, 4);
    layout->addWidget(m_radius, 0, 5);
    layout->addWidget(new QLabel("Старт:"), 0, 6);
    layout->addWidget(m_startAngle, 0, 7);
    
    layout->addWidget(new QLabel("Сектор:"), 1, 0);
    layout->addWidget(m_spanAngle, 1, 1);

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
        m_centerX->setText(QString::number(m_currentArc->getCenter().getX(), 'f', 2));
        m_centerY->setText(QString::number(m_currentArc->getCenter().getY(), 'f', 2));
        m_radius->setText(QString::number(m_currentArc->getRadius(), 'f', 2));
        m_startAngle->setText(QString::number(m_currentArc->getStartAngle(), 'f', 2));
        m_spanAngle->setText(QString::number(m_currentArc->getSpanAngle(), 'f', 2));
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
