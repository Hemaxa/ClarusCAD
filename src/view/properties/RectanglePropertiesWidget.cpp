#include "RectanglePropertiesWidget.h"
#include "RectanglePrimitive.h"
#include <QGridLayout>
#include <QVBoxLayout>
#include <QDoubleValidator>
#include <QPushButton>
#include <QLabel>
#include <QtMath>

RectanglePropertiesWidget::RectanglePropertiesWidget(QWidget* parent) : BasePropertiesWidget(parent)
{
    auto* validator = new QDoubleValidator(this);
    auto* layout = static_cast<QGridLayout*>(m_cartesianWidgets->layout());

    //выбор режима
    layout->addWidget(new QLabel("Режим:"), 0, 0);
    m_modeComboBox = new QComboBox();
    m_modeComboBox->addItem("2 точки");
    m_modeComboBox->addItem("Центр+размер");
    m_modeComboBox->addItem("3 точки");
    m_modeComboBox->setObjectName("PropertiesComboBox");
    connect(m_modeComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), 
            this, &RectanglePropertiesWidget::onModeChanged);
    layout->addWidget(m_modeComboBox, 0, 1, 1, 3);

    //центр/размер
    m_centerX = new QLineEdit("0.0"); m_centerX->setValidator(validator); m_centerX->setObjectName("PropertiesInput");
    m_centerY = new QLineEdit("0.0"); m_centerY->setValidator(validator); m_centerY->setObjectName("PropertiesInput");
    m_width = new QLineEdit("0.0"); m_width->setValidator(validator); m_width->setObjectName("PropertiesInput");
    m_height = new QLineEdit("0.0"); m_height->setValidator(validator); m_height->setObjectName("PropertiesInput");
    m_rotation = new QLineEdit("0.0"); m_rotation->setValidator(validator); m_rotation->setObjectName("PropertiesInput");

    layout->addWidget(new QLabel("X:"), 1, 0);
    layout->addWidget(m_centerX, 1, 1);
    layout->addWidget(new QLabel("Y:"), 1, 2);
    layout->addWidget(m_centerY, 1, 3);
    
    layout->addWidget(new QLabel("W:"), 2, 0);
    layout->addWidget(m_width, 2, 1);
    layout->addWidget(new QLabel("H:"), 2, 2);
    layout->addWidget(m_height, 2, 3);
    
    layout->addWidget(new QLabel("Угол:"), 3, 0);
    layout->addWidget(m_rotation, 3, 1);

    //скругление
    m_cornerTypeCombo = new QComboBox();
    m_cornerTypeCombo->addItem("Без скр.", static_cast<int>(CornerType::None));
    m_cornerTypeCombo->addItem("Скругл.", static_cast<int>(CornerType::Fillet));
    m_cornerTypeCombo->addItem("Фаска", static_cast<int>(CornerType::Chamfer));
    m_cornerTypeCombo->setObjectName("PropertiesComboBox");
    m_cornerRadiusEdit = new QLineEdit("0.0");
    m_cornerRadiusEdit->setValidator(validator);
    m_cornerRadiusEdit->setObjectName("PropertiesInput");

    layout->addWidget(new QLabel("Угол:"), 3, 2);
    layout->addWidget(m_cornerTypeCombo, 3, 3);
    layout->addWidget(new QLabel("R угла:"), 4, 0);
    layout->addWidget(m_cornerRadiusEdit, 4, 1);

    //полярные координаты - минимальные
    auto* polarLayout = static_cast<QGridLayout*>(m_polarWidgets->layout());
    m_centerRadius = new QLineEdit("0.0"); m_centerRadius->setValidator(validator); m_centerRadius->setObjectName("PropertiesInput");
    m_centerAngle  = new QLineEdit("0.0"); m_centerAngle->setValidator(validator); m_centerAngle->setObjectName("PropertiesInput");
    polarLayout->addWidget(new QLabel("R:"), 0, 0);
    polarLayout->addWidget(m_centerRadius, 0, 1);
    polarLayout->addWidget(new QLabel("A:"), 0, 2);
    polarLayout->addWidget(m_centerAngle, 0, 3);

    connect(m_applyButton, &QPushButton::clicked, this, &RectanglePropertiesWidget::onApplyButtonClicked);
}

void RectanglePropertiesWidget::onModeChanged(int index)
{
    Q_UNUSED(index);
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
        PointPrimitive center = m_currentRect->getCenter();

        m_centerX->setText(QString::number(center.getX(), 'f', 2));
        m_centerY->setText(QString::number(center.getY(), 'f', 2));
        m_centerRadius->setText(QString::number(center.getRadius(), 'f', 2));
        m_centerAngle->setText(QString::number(center.getAngle(), 'f', 2));

        m_width->setText(QString::number(m_currentRect->getWidth(), 'f', 2));
        m_height->setText(QString::number(m_currentRect->getHeight(), 'f', 2));
        m_rotation->setText(QString::number(m_currentRect->getRotation(), 'f', 2));

        int cornerIdx = m_cornerTypeCombo->findData(static_cast<int>(m_currentRect->getCornerType()));
        if (cornerIdx >= 0) m_cornerTypeCombo->setCurrentIndex(cornerIdx);
        m_cornerRadiusEdit->setText(QString::number(m_currentRect->getCornerRadius(), 'f', 2));
    } else {
        m_centerX->setText("0.0"); m_centerY->setText("0.0");
        m_centerRadius->setText("0.0"); m_centerAngle->setText("0.0");
        m_width->setText("0.0"); m_height->setText("0.0"); m_rotation->setText("0.0");
        m_cornerTypeCombo->setCurrentIndex(0);
        m_cornerRadiusEdit->setText("0.0");
    }
}

void RectanglePropertiesWidget::onApplyButtonClicked()
{
    PointPrimitive center;

    if (m_selectedCoordSystem == CoordinateSystemType::Cartesian) {
        center.setX(m_centerX->text().toDouble());
        center.setY(m_centerY->text().toDouble());
    } else {
        center.setPolar(m_centerRadius->text().toDouble(), m_centerAngle->text().toDouble());
    }

    double w = m_width->text().toDouble();
    double h = m_height->text().toDouble();
    double r = m_rotation->text().toDouble();

    CornerType cornerType = static_cast<CornerType>(m_cornerTypeCombo->currentData().toInt());
    double cornerRadius = m_cornerRadiusEdit->text().toDouble();

    int typeEmit = m_selectedLineTypeId;
    if(typeEmit == -1 && m_currentRect) typeEmit = m_currentRect->getLineType();
    if(typeEmit == -1) typeEmit = (int)LineType::SolidMain;

    emit propertiesApplied(m_currentRect, center, w, h, r, cornerType, cornerRadius, 
                           m_selectedColor, (LineType)typeEmit);
}
