#include "RectanglePropertiesWidget.h"
#include "RectanglePrimitive.h"
#include <QFormLayout>
#include <QDoubleValidator>
#include <QPushButton>
#include <QVBoxLayout>
#include <QtMath>

RectanglePropertiesWidget::RectanglePropertiesWidget(QWidget* parent) : BasePropertiesWidget(parent)
{
    auto* validator = new QDoubleValidator(this);

    // --- Выбор способа построения ---
    auto* mainLayout = static_cast<QFormLayout*>(m_cartesianWidgets->layout());
    
    m_modeComboBox = new QComboBox();
    m_modeComboBox->addItem("Две точки");
    m_modeComboBox->addItem("Центр и размер");
    m_modeComboBox->addItem("Три точки");
    m_modeComboBox->setObjectName("PropertiesComboBox");
    connect(m_modeComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), 
            this, &RectanglePropertiesWidget::onModeChanged);
    mainLayout->addRow("Способ задания:", m_modeComboBox);

    // --- Декартовы координаты ---
    m_centerX = new QLineEdit("0.0"); 
    m_centerX->setValidator(validator);
    m_centerX->setObjectName("PropertiesInput");
    
    m_centerY = new QLineEdit("0.0"); 
    m_centerY->setValidator(validator);
    m_centerY->setObjectName("PropertiesInput");

    mainLayout->addRow("Центр X:", m_centerX);
    mainLayout->addRow("Центр Y:", m_centerY);

    // --- Полярные координаты ---
    auto* polarLayout = static_cast<QFormLayout*>(m_polarWidgets->layout());
    m_centerRadius = new QLineEdit("0.0"); 
    m_centerRadius->setValidator(validator);
    m_centerRadius->setObjectName("PropertiesInput");
    
    m_centerAngle  = new QLineEdit("0.0"); 
    m_centerAngle->setValidator(validator);
    m_centerAngle->setObjectName("PropertiesInput");

    polarLayout->addRow("Центр R:", m_centerRadius);
    polarLayout->addRow("Центр A:", m_centerAngle);

    // --- Общие параметры ---
    auto* centralLayout = static_cast<QVBoxLayout*>(m_centralColumn->layout());

    QWidget* commonWidget = new QWidget();
    auto* commonLayout = new QFormLayout(commonWidget);
    commonLayout->setContentsMargins(0, 10, 10, 0);

    m_width = new QLineEdit("0.0"); 
    m_width->setValidator(validator);
    m_width->setObjectName("PropertiesInput");
    
    m_height = new QLineEdit("0.0"); 
    m_height->setValidator(validator);
    m_height->setObjectName("PropertiesInput");
    
    m_rotation = new QLineEdit("0.0"); 
    m_rotation->setValidator(validator);
    m_rotation->setObjectName("PropertiesInput");

    commonLayout->addRow("Ширина:", m_width);
    commonLayout->addRow("Высота:", m_height);
    commonLayout->addRow("Поворот (°):", m_rotation);

    // --- Скругление углов ---
    m_cornerTypeCombo = new QComboBox();
    m_cornerTypeCombo->addItem("Без скругления", static_cast<int>(CornerType::None));
    m_cornerTypeCombo->addItem("Скругление", static_cast<int>(CornerType::Fillet));
    m_cornerTypeCombo->addItem("Фаска", static_cast<int>(CornerType::Chamfer));
    m_cornerTypeCombo->setObjectName("PropertiesComboBox");
    
    m_cornerRadiusEdit = new QLineEdit("0.0");
    m_cornerRadiusEdit->setValidator(validator);
    m_cornerRadiusEdit->setObjectName("PropertiesInput");

    commonLayout->addRow("Тип угла:", m_cornerTypeCombo);
    commonLayout->addRow("Радиус:", m_cornerRadiusEdit);

    centralLayout->addWidget(commonWidget);

    connect(m_applyButton, &QPushButton::clicked, this, &RectanglePropertiesWidget::onApplyButtonClicked);
}

void RectanglePropertiesWidget::onModeChanged(int index)
{
    Q_UNUSED(index);
    // Можно добавить логику переключения полей для разных режимов
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

        // Декартовы
        m_centerX->setText(QString::number(center.getX()));
        m_centerY->setText(QString::number(center.getY()));

        // Полярные
        m_centerRadius->setText(QString::number(center.getRadius()));
        m_centerAngle->setText(QString::number(center.getAngle()));

        // Общие
        m_width->setText(QString::number(m_currentRect->getWidth()));
        m_height->setText(QString::number(m_currentRect->getHeight()));
        m_rotation->setText(QString::number(m_currentRect->getRotation()));

        // Скругление
        int cornerIdx = m_cornerTypeCombo->findData(static_cast<int>(m_currentRect->getCornerType()));
        if (cornerIdx >= 0) m_cornerTypeCombo->setCurrentIndex(cornerIdx);
        m_cornerRadiusEdit->setText(QString::number(m_currentRect->getCornerRadius()));
    } else {
        // Дефолт для создания
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
