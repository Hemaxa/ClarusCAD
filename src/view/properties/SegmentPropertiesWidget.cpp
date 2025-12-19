#include "SegmentPropertiesWidget.h"
#include "SegmentPrimitive.h"
#include "ThemeManager.h"

#include <QGridLayout>
#include <QStackedWidget>
#include <QLineEdit>
#include <QDoubleValidator>
#include <QPushButton>
#include <QLabel>

SegmentPropertiesWidget::SegmentPropertiesWidget(QWidget* parent) : BasePropertiesWidget(parent)
{
    auto* validator = new QDoubleValidator(this);

    //получаем grid layout декартовых координат
    auto* cartesianLayout = static_cast<QGridLayout*>(m_cartesianWidgets->layout());
    
    //создаём поля
    m_startXEdit = new QLineEdit("0.0");
    m_startYEdit = new QLineEdit("0.0");
    m_endXEdit = new QLineEdit("0.0");
    m_endYEdit = new QLineEdit("0.0");
    m_startXEdit->setValidator(validator);
    m_startYEdit->setValidator(validator);
    m_endXEdit->setValidator(validator);
    m_endYEdit->setValidator(validator);
    m_startXEdit->setObjectName("PropertiesInput");
    m_startYEdit->setObjectName("PropertiesInput");
    m_endXEdit->setObjectName("PropertiesInput");
    m_endYEdit->setObjectName("PropertiesInput");

    //компактное размещение: 2 ряда по 4 элемента
    //ряд 0: X1, поле, Y1, поле
    cartesianLayout->addWidget(new QLabel("X1:"), 0, 0);
    cartesianLayout->addWidget(m_startXEdit, 0, 1);
    cartesianLayout->addWidget(new QLabel("Y1:"), 0, 2);
    cartesianLayout->addWidget(m_startYEdit, 0, 3);
    
    //ряд 1: X2, поле, Y2, поле
    cartesianLayout->addWidget(new QLabel("X2:"), 1, 0);
    cartesianLayout->addWidget(m_endXEdit, 1, 1);
    cartesianLayout->addWidget(new QLabel("Y2:"), 1, 2);
    cartesianLayout->addWidget(m_endYEdit, 1, 3);

    //полярные координаты
    auto* polarLayout = static_cast<QGridLayout*>(m_polarWidgets->layout());
    m_startRadiusEdit = new QLineEdit("0.0");
    m_startAngleEdit = new QLineEdit("0.0");
    m_endRadiusEdit = new QLineEdit("0.0");
    m_endAngleEdit = new QLineEdit("0.0");
    m_startRadiusEdit->setValidator(validator);
    m_startAngleEdit->setValidator(validator);
    m_endRadiusEdit->setValidator(validator);
    m_endAngleEdit->setValidator(validator);
    m_startRadiusEdit->setObjectName("PropertiesInput");
    m_startAngleEdit->setObjectName("PropertiesInput");
    m_endRadiusEdit->setObjectName("PropertiesInput");
    m_endAngleEdit->setObjectName("PropertiesInput");

    polarLayout->addWidget(new QLabel("R1:"), 0, 0);
    polarLayout->addWidget(m_startRadiusEdit, 0, 1);
    polarLayout->addWidget(new QLabel("A1:"), 0, 2);
    polarLayout->addWidget(m_startAngleEdit, 0, 3);
    
    polarLayout->addWidget(new QLabel("R2:"), 1, 0);
    polarLayout->addWidget(m_endRadiusEdit, 1, 1);
    polarLayout->addWidget(new QLabel("A2:"), 1, 2);
    polarLayout->addWidget(m_endAngleEdit, 1, 3);

    connect(m_applyButton, &QPushButton::clicked, this, &SegmentPropertiesWidget::onApplyButtonClicked);

    setCoordinateSystem(CoordinateSystemType::Cartesian);
}

void SegmentPropertiesWidget::setPrimitives(const QList<BasePrimitive*>& primitives)
{
    BasePropertiesWidget::setPrimitives(primitives);

    m_currentSegment = nullptr;
    if (m_currentPrimitive) {
        m_currentSegment = dynamic_cast<SegmentPrimitive*>(m_currentPrimitive);
    }

    updateFieldValues();
}

void SegmentPropertiesWidget::updateFieldValues()
{
    bool isCartesian = (m_selectedCoordSystem == CoordinateSystemType::Cartesian);

    if (m_currentSegment) {
        const auto& startPoint = m_currentSegment->getStart();
        const auto& endPoint = m_currentSegment->getEnd();

        if (isCartesian) {
            m_startXEdit->setText(QString::number(startPoint.getX(), 'f', 2));
            m_startYEdit->setText(QString::number(startPoint.getY(), 'f', 2));
            m_endXEdit->setText(QString::number(endPoint.getX(), 'f', 2));
            m_endYEdit->setText(QString::number(endPoint.getY(), 'f', 2));
        } else {
            m_startRadiusEdit->setText(QString::number(startPoint.getRadius(), 'f', 2));
            m_startAngleEdit->setText(QString::number(startPoint.getAngle(), 'f', 2));
            m_endRadiusEdit->setText(QString::number(endPoint.getRadius(), 'f', 2));
            m_endAngleEdit->setText(QString::number(endPoint.getAngle(), 'f', 2));
        }
    }
    else {
        m_startXEdit->setText("0.0");
        m_startYEdit->setText("0.0");
        m_endXEdit->setText("0.0");
        m_endYEdit->setText("0.0");
        m_startRadiusEdit->setText("0.0");
        m_startAngleEdit->setText("0.0");
        m_endRadiusEdit->setText("0.0");
        m_endAngleEdit->setText("0.0");
    }
}

void SegmentPropertiesWidget::onApplyButtonClicked()
{
    PointPrimitive start, end;
    bool isCartesian = (m_selectedCoordSystem == CoordinateSystemType::Cartesian);

    if (isCartesian) {
        start.setX(m_startXEdit->text().toDouble());
        start.setY(m_startYEdit->text().toDouble());
        end.setX(m_endXEdit->text().toDouble());
        end.setY(m_endYEdit->text().toDouble());
    }
    else {
        start.setPolar(m_startRadiusEdit->text().toDouble(), m_startAngleEdit->text().toDouble());
        end.setPolar(m_endRadiusEdit->text().toDouble(), m_endAngleEdit->text().toDouble());
    }

    int typeToEmit = m_selectedLineTypeId;
    if(typeToEmit == -1 && m_currentSegment) typeToEmit = m_currentSegment->getLineType();
    if(typeToEmit == -1) typeToEmit = (int)LineType::SolidMain;

    emit propertiesApplied(m_currentSegment, start, end, m_selectedColor, static_cast<LineType>(typeToEmit));
}
