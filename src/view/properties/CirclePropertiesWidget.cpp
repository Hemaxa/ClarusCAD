#include "CirclePropertiesWidget.h"
#include "CirclePrimitive.h"
#include "ThemeManager.h"

#include <QGridLayout>
#include <QVBoxLayout>
#include <QDoubleValidator>
#include <QPushButton>
#include <QComboBox>
#include <QLabel>
#include <QtMath>
#include <QMessageBox>

CirclePropertiesWidget::CirclePropertiesWidget(QWidget* parent) : BasePropertiesWidget(parent)
{
    auto* validator = new QDoubleValidator(this);

    //получаем grid layout декартовых координат
    auto* mainLayout = static_cast<QGridLayout*>(m_cartesianWidgets->layout());

    //выбор режима в строке 0
    mainLayout->addWidget(new QLabel("Режим:"), 0, 0);
    m_modeComboBox = new QComboBox();
    m_modeComboBox->addItem("Центр+R");
    m_modeComboBox->addItem("Центр+D");
    m_modeComboBox->addItem("2 точки");
    m_modeComboBox->addItem("3 точки");
    m_modeComboBox->setObjectName("PropertiesComboBox");
    connect(m_modeComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &CirclePropertiesWidget::onModeChanged);
    mainLayout->addWidget(m_modeComboBox, 0, 1, 1, 3);

    //StackedWidget для смены полей
    m_modeStack = new QStackedWidget();

    // --- СТРАНИЦА 1: Центр + Радиус (компактно: 3 пары в ряд) ---
    m_pageCenterRadius = new QWidget();
    auto* l1 = new QGridLayout(m_pageCenterRadius);
    l1->setContentsMargins(0,4,0,0);
    l1->setSpacing(4);
    m_crCenterX = new QLineEdit("0.0"); m_crCenterX->setValidator(validator); m_crCenterX->setObjectName("PropertiesInput");
    m_crCenterY = new QLineEdit("0.0"); m_crCenterY->setValidator(validator); m_crCenterY->setObjectName("PropertiesInput");
    m_crRadius  = new QLineEdit("0.0"); m_crRadius->setValidator(validator); m_crRadius->setObjectName("PropertiesInput");
    l1->addWidget(new QLabel("X:"), 0, 0);
    l1->addWidget(m_crCenterX, 0, 1);
    l1->addWidget(new QLabel("Y:"), 0, 2);
    l1->addWidget(m_crCenterY, 0, 3);
    l1->addWidget(new QLabel("R:"), 0, 4);
    l1->addWidget(m_crRadius, 0, 5);
    m_modeStack->addWidget(m_pageCenterRadius);

    // --- СТРАНИЦА 2: Центр + Диаметр ---
    m_pageCenterDiameter = new QWidget();
    auto* l2 = new QGridLayout(m_pageCenterDiameter);
    l2->setContentsMargins(0,4,0,0);
    l2->setSpacing(4);
    m_cdCenterX = new QLineEdit("0.0"); m_cdCenterX->setValidator(validator); m_cdCenterX->setObjectName("PropertiesInput");
    m_cdCenterY = new QLineEdit("0.0"); m_cdCenterY->setValidator(validator); m_cdCenterY->setObjectName("PropertiesInput");
    m_cdDiameter = new QLineEdit("0.0"); m_cdDiameter->setValidator(validator); m_cdDiameter->setObjectName("PropertiesInput");
    l2->addWidget(new QLabel("X:"), 0, 0);
    l2->addWidget(m_cdCenterX, 0, 1);
    l2->addWidget(new QLabel("Y:"), 0, 2);
    l2->addWidget(m_cdCenterY, 0, 3);
    l2->addWidget(new QLabel("D:"), 0, 4);
    l2->addWidget(m_cdDiameter, 0, 5);
    m_modeStack->addWidget(m_pageCenterDiameter);

    // --- СТРАНИЦА 3: Две точки ---
    m_pageTwoPoints = new QWidget();
    auto* l3 = new QGridLayout(m_pageTwoPoints);
    l3->setContentsMargins(0,4,0,0);
    l3->setSpacing(4);
    m_tpX1 = new QLineEdit("0.0"); m_tpX1->setValidator(validator); m_tpX1->setObjectName("PropertiesInput");
    m_tpY1 = new QLineEdit("0.0"); m_tpY1->setValidator(validator); m_tpY1->setObjectName("PropertiesInput");
    m_tpX2 = new QLineEdit("0.0"); m_tpX2->setValidator(validator); m_tpX2->setObjectName("PropertiesInput");
    m_tpY2 = new QLineEdit("0.0"); m_tpY2->setValidator(validator); m_tpY2->setObjectName("PropertiesInput");
    l3->addWidget(new QLabel("X1:"), 0, 0);
    l3->addWidget(m_tpX1, 0, 1);
    l3->addWidget(new QLabel("Y1:"), 0, 2);
    l3->addWidget(m_tpY1, 0, 3);
    l3->addWidget(new QLabel("X2:"), 0, 4);
    l3->addWidget(m_tpX2, 0, 5);
    l3->addWidget(new QLabel("Y2:"), 0, 6);
    l3->addWidget(m_tpY2, 0, 7);
    m_modeStack->addWidget(m_pageTwoPoints);

    // --- СТРАНИЦА 4: Три точки ---
    m_pageThreePoints = new QWidget();
    auto* l4 = new QGridLayout(m_pageThreePoints);
    l4->setContentsMargins(0,4,0,0);
    l4->setSpacing(4);
    m_thpX1 = new QLineEdit("0.0"); m_thpX1->setValidator(validator); m_thpX1->setObjectName("PropertiesInput");
    m_thpY1 = new QLineEdit("0.0"); m_thpY1->setValidator(validator); m_thpY1->setObjectName("PropertiesInput");
    m_thpX2 = new QLineEdit("0.0"); m_thpX2->setValidator(validator); m_thpX2->setObjectName("PropertiesInput");
    m_thpY2 = new QLineEdit("0.0"); m_thpY2->setValidator(validator); m_thpY2->setObjectName("PropertiesInput");
    m_thpX3 = new QLineEdit("0.0"); m_thpX3->setValidator(validator); m_thpX3->setObjectName("PropertiesInput");
    m_thpY3 = new QLineEdit("0.0"); m_thpY3->setValidator(validator); m_thpY3->setObjectName("PropertiesInput");
    l4->addWidget(new QLabel("X1:"), 0, 0);
    l4->addWidget(m_thpX1, 0, 1);
    l4->addWidget(new QLabel("Y1:"), 0, 2);
    l4->addWidget(m_thpY1, 0, 3);
    l4->addWidget(new QLabel("X2:"), 0, 4);
    l4->addWidget(m_thpX2, 0, 5);
    l4->addWidget(new QLabel("Y2:"), 0, 6);
    l4->addWidget(m_thpY2, 0, 7);
    l4->addWidget(new QLabel("X3:"), 1, 0);
    l4->addWidget(m_thpX3, 1, 1);
    l4->addWidget(new QLabel("Y3:"), 1, 2);
    l4->addWidget(m_thpY3, 1, 3);
    m_modeStack->addWidget(m_pageThreePoints);

    mainLayout->addWidget(m_modeStack, 1, 0, 1, 4);

    connect(m_applyButton, &QPushButton::clicked, this, &CirclePropertiesWidget::onApplyButtonClicked);

    setCoordinateSystem(CoordinateSystemType::Cartesian);
}

void CirclePropertiesWidget::setPrimitives(const QList<BasePrimitive*>& primitives)
{
    BasePropertiesWidget::setPrimitives(primitives);

    m_currentCircle = nullptr;
    if (m_currentPrimitive && m_currentPrimitive->getType() == PrimitiveType::Circle) {
        m_currentCircle = static_cast<CirclePrimitive*>(m_currentPrimitive);
    }

    m_modeComboBox->setCurrentIndex(0);
    updateFieldValues();
}

void CirclePropertiesWidget::onModeChanged(int index)
{
    m_modeStack->setCurrentIndex(index);
}

void CirclePropertiesWidget::updateFieldValues()
{
    if (m_currentCircle) {
        PointPrimitive center = m_currentCircle->getCenter();
        double r = m_currentCircle->getRadius();

        m_crCenterX->setText(QString::number(center.getX(), 'f', 2));
        m_crCenterY->setText(QString::number(center.getY(), 'f', 2));
        m_crRadius->setText(QString::number(r, 'f', 2));

        m_cdCenterX->setText(QString::number(center.getX(), 'f', 2));
        m_cdCenterY->setText(QString::number(center.getY(), 'f', 2));
        m_cdDiameter->setText(QString::number(r * 2.0, 'f', 2));
    } else {
        m_crCenterX->setText("0.0"); m_crCenterY->setText("0.0"); m_crRadius->setText("0.0");
    }
}

void CirclePropertiesWidget::onCartesianRadiusChanged(const QString& text) {}
void CirclePropertiesWidget::onCartesianDiameterChanged(const QString& text) {}

void CirclePropertiesWidget::onApplyButtonClicked()
{
    PointPrimitive center;
    double radius = 0.0;

    int mode = m_modeComboBox->currentIndex();
    bool isValid = true;

    if (mode == 0) {
        center.setX(m_crCenterX->text().toDouble());
        center.setY(m_crCenterY->text().toDouble());
        radius = m_crRadius->text().toDouble();
    }
    else if (mode == 1) {
        center.setX(m_cdCenterX->text().toDouble());
        center.setY(m_cdCenterY->text().toDouble());
        radius = m_cdDiameter->text().toDouble() / 2.0;
    }
    else if (mode == 2) {
        double x1 = m_tpX1->text().toDouble();
        double y1 = m_tpY1->text().toDouble();
        double x2 = m_tpX2->text().toDouble();
        double y2 = m_tpY2->text().toDouble();

        center.setX((x1 + x2) / 2.0);
        center.setY((y1 + y2) / 2.0);
        radius = std::sqrt(std::pow(x2 - x1, 2) + std::pow(y2 - y1, 2)) / 2.0;
    }
    else if (mode == 3) {
        PointPrimitive p1(m_thpX1->text().toDouble(), m_thpY1->text().toDouble());
        PointPrimitive p2(m_thpX2->text().toDouble(), m_thpY2->text().toDouble());
        PointPrimitive p3(m_thpX3->text().toDouble(), m_thpY3->text().toDouble());

        if (!getCircleFrom3Points(p1, p2, p3, center, radius)) {
            QMessageBox::warning(this, "Ошибка", "Невозможно построить окружность (точки на одной прямой).");
            isValid = false;
        }
    }

    if (isValid && radius >= 0) {
        int typeToEmit = m_selectedLineTypeId;
        if(typeToEmit == -1 && m_currentCircle) typeToEmit = m_currentCircle->getLineType();
        if(typeToEmit == -1) typeToEmit = (int)LineType::SolidMain;

        emit propertiesApplied(m_currentCircle, center, radius, m_selectedColor, static_cast<LineType>(typeToEmit));
    }
}

bool CirclePropertiesWidget::getCircleFrom3Points(const PointPrimitive& p1, const PointPrimitive& p2, const PointPrimitive& p3, PointPrimitive& center, double& radius)
{
    double x1 = p1.getX(), y1 = p1.getY();
    double x2 = p2.getX(), y2 = p2.getY();
    double x3 = p3.getX(), y3 = p3.getY();

    double D = 2 * (x1 * (y2 - y3) + x2 * (y3 - y1) + x3 * (y1 - y2));

    if (std::abs(D) < 1e-9) return false;

    double centerX = ((x1 * x1 + y1 * y1) * (y2 - y3) + (x2 * x2 + y2 * y2) * (y3 - y1) + (x3 * x3 + y3 * y3) * (y1 - y2)) / D;
    double centerY = ((x1 * x1 + y1 * y1) * (x3 - x2) + (x2 * x2 + y2 * y2) * (x1 - x3) + (x3 * x3 + y3 * y3) * (x2 - x1)) / D;

    center.setX(centerX);
    center.setY(centerY);

    radius = std::sqrt(std::pow(centerX - x1, 2) + std::pow(centerY - y1, 2));
    return true;
}
