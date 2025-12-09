#include "EllipsePropertiesWidget.h"
#include "EllipsePrimitive.h"
#include <QFormLayout>
#include <QDoubleValidator>
#include <QPushButton>

EllipsePropertiesWidget::EllipsePropertiesWidget(QWidget* parent) : BasePropertiesWidget(parent)
{
    auto* validator = new QDoubleValidator(this);
    auto* layout = static_cast<QFormLayout*>(m_cartesianWidgets->layout());

    m_centerX = new QLineEdit("0.0"); m_centerX->setValidator(validator);
    m_centerY = new QLineEdit("0.0"); m_centerY->setValidator(validator);
    m_radiusX = new QLineEdit("0.0"); m_radiusX->setValidator(validator);
    m_radiusY = new QLineEdit("0.0"); m_radiusY->setValidator(validator);
    m_rotation = new QLineEdit("0.0"); m_rotation->setValidator(validator);

    layout->addRow("Центр X:", m_centerX);
    layout->addRow("Центр Y:", m_centerY);
    layout->addRow("Радиус X:", m_radiusX);
    layout->addRow("Радиус Y:", m_radiusY);
    layout->addRow("Поворот:", m_rotation);

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
        m_centerX->setText(QString::number(m_currentEllipse->getCenter().getX()));
        m_centerY->setText(QString::number(m_currentEllipse->getCenter().getY()));
        m_radiusX->setText(QString::number(m_currentEllipse->getRadiusX()));
        m_radiusY->setText(QString::number(m_currentEllipse->getRadiusY()));
        m_rotation->setText(QString::number(m_currentEllipse->getRotation()));
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
