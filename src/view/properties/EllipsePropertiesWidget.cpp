#include "EllipsePropertiesWidget.h"
#include "EllipsePrimitive.h"
#include <QGridLayout>
#include <QDoubleValidator>
#include <QPushButton>
#include <QLabel>

EllipsePropertiesWidget::EllipsePropertiesWidget(QWidget* parent) : BasePropertiesWidget(parent)
{
    auto* validator = new QDoubleValidator(this);
    auto* layout = static_cast<QGridLayout*>(m_cartesianWidgets->layout());

    m_centerX = new QLineEdit("0.0"); m_centerX->setValidator(validator); m_centerX->setObjectName("PropertiesInput");
    m_centerY = new QLineEdit("0.0"); m_centerY->setValidator(validator); m_centerY->setObjectName("PropertiesInput");
    m_radiusX = new QLineEdit("0.0"); m_radiusX->setValidator(validator); m_radiusX->setObjectName("PropertiesInput");
    m_radiusY = new QLineEdit("0.0"); m_radiusY->setValidator(validator); m_radiusY->setObjectName("PropertiesInput");
    m_rotation = new QLineEdit("0.0"); m_rotation->setValidator(validator); m_rotation->setObjectName("PropertiesInput");

    //компактный grid: 1 ряд по 4 элемента (4 пары)
    layout->addWidget(new QLabel("X:"), 0, 0);
    layout->addWidget(m_centerX, 0, 1);
    layout->addWidget(new QLabel("Y:"), 0, 2);
    layout->addWidget(m_centerY, 0, 3);
    layout->addWidget(new QLabel("RX:"), 0, 4);
    layout->addWidget(m_radiusX, 0, 5);
    layout->addWidget(new QLabel("RY:"), 0, 6);
    layout->addWidget(m_radiusY, 0, 7);
    
    layout->addWidget(new QLabel("Угол:"), 1, 0);
    layout->addWidget(m_rotation, 1, 1);

    connect(m_applyButton, &QPushButton::clicked, this, &EllipsePropertiesWidget::onApplyButtonClicked);
}

void EllipsePropertiesWidget::setPrimitives(const QList<BasePrimitive*>& primitives)
{
    BasePropertiesWidget::setPrimitives(primitives);
    m_currentEllipse = nullptr;
    if(m_currentPrimitive && m_currentPrimitive->getType() == PrimitiveType::Ellipse) {
        m_currentEllipse = static_cast<EllipsePrimitive*>(m_currentPrimitive);
    }
    updateFieldValues();
}

void EllipsePropertiesWidget::updateFieldValues()
{
    if(m_currentEllipse) {
        m_centerX->setText(QString::number(m_currentEllipse->getCenter().getX(), 'f', 2));
        m_centerY->setText(QString::number(m_currentEllipse->getCenter().getY(), 'f', 2));
        m_radiusX->setText(QString::number(m_currentEllipse->getRadiusX(), 'f', 2));
        m_radiusY->setText(QString::number(m_currentEllipse->getRadiusY(), 'f', 2));
        m_rotation->setText(QString::number(m_currentEllipse->getRotation(), 'f', 2));
    }
}

void EllipsePropertiesWidget::onApplyButtonClicked()
{
    double cx = m_centerX->text().toDouble();
    double cy = m_centerY->text().toDouble();
    double rx = m_radiusX->text().toDouble();
    double ry = m_radiusY->text().toDouble();
    double rot = m_rotation->text().toDouble();

    int typeEmit = m_selectedLineTypeId;
    if(typeEmit == -1 && m_currentEllipse) typeEmit = m_currentEllipse->getLineType();
    if(typeEmit == -1) typeEmit = (int)LineType::SolidMain;

    emit propertiesApplied(m_currentEllipse, PointPrimitive(cx, cy), rx, ry, rot, m_selectedColor, (LineType)typeEmit);
}
