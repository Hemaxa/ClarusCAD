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
    m_modeComboBox->addItem("2 точки", static_cast<int>(RectangleCreationMode::TwoPoints));
    m_modeComboBox->addItem("Центр+размер", static_cast<int>(RectangleCreationMode::CenterSize));
    m_modeComboBox->addItem("Точка+размер", static_cast<int>(RectangleCreationMode::PointSize));
    m_modeComboBox->setObjectName("PropertiesComboBox");
    connect(m_modeComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), 
            this, &RectanglePropertiesWidget::onModeChanged);
    layout->addWidget(m_modeComboBox, 0, 1, 1, 7);

    //StackedWidget для переключаемых полей
    m_modeStack = new QStackedWidget();

    // --- СТРАНИЦА 0: Две точки (P1, P2) ---
    auto* pageTwoPoints = new QWidget();
    auto* l0 = new QGridLayout(pageTwoPoints);
    l0->setContentsMargins(0, 4, 0, 0);
    l0->setSpacing(4);
    m_p1X = new QLineEdit("0.0"); m_p1X->setValidator(validator); m_p1X->setObjectName("PropertiesInput");
    m_p1Y = new QLineEdit("0.0"); m_p1Y->setValidator(validator); m_p1Y->setObjectName("PropertiesInput");
    m_p2X = new QLineEdit("0.0"); m_p2X->setValidator(validator); m_p2X->setObjectName("PropertiesInput");
    m_p2Y = new QLineEdit("0.0"); m_p2Y->setValidator(validator); m_p2Y->setObjectName("PropertiesInput");
    l0->addWidget(new QLabel("X1:"), 0, 0);
    l0->addWidget(m_p1X, 0, 1);
    l0->addWidget(new QLabel("Y1:"), 0, 2);
    l0->addWidget(m_p1Y, 0, 3);
    l0->addWidget(new QLabel("X2:"), 0, 4);
    l0->addWidget(m_p2X, 0, 5);
    l0->addWidget(new QLabel("Y2:"), 0, 6);
    l0->addWidget(m_p2Y, 0, 7);
    m_modeStack->addWidget(pageTwoPoints);

    // --- СТРАНИЦА 1: Центр + размер ---
    auto* pageCenterSize = new QWidget();
    auto* l1 = new QGridLayout(pageCenterSize);
    l1->setContentsMargins(0, 4, 0, 0);
    l1->setSpacing(4);
    m_centerX = new QLineEdit("0.0"); m_centerX->setValidator(validator); m_centerX->setObjectName("PropertiesInput");
    m_centerY = new QLineEdit("0.0"); m_centerY->setValidator(validator); m_centerY->setObjectName("PropertiesInput");
    m_width = new QLineEdit("0.0"); m_width->setValidator(validator); m_width->setObjectName("PropertiesInput");
    m_height = new QLineEdit("0.0"); m_height->setValidator(validator); m_height->setObjectName("PropertiesInput");
    l1->addWidget(new QLabel("X:"), 0, 0);
    l1->addWidget(m_centerX, 0, 1);
    l1->addWidget(new QLabel("Y:"), 0, 2);
    l1->addWidget(m_centerY, 0, 3);
    l1->addWidget(new QLabel("W:"), 0, 4);
    l1->addWidget(m_width, 0, 5);
    l1->addWidget(new QLabel("H:"), 0, 6);
    l1->addWidget(m_height, 0, 7);
    m_modeStack->addWidget(pageCenterSize);

    // --- СТРАНИЦА 2: Точка + размер ---
    auto* pagePointSize = new QWidget();
    auto* l2 = new QGridLayout(pagePointSize);
    l2->setContentsMargins(0, 4, 0, 0);
    l2->setSpacing(4);
    m_pointX = new QLineEdit("0.0"); m_pointX->setValidator(validator); m_pointX->setObjectName("PropertiesInput");
    m_pointY = new QLineEdit("0.0"); m_pointY->setValidator(validator); m_pointY->setObjectName("PropertiesInput");
    m_widthPS = new QLineEdit("0.0"); m_widthPS->setValidator(validator); m_widthPS->setObjectName("PropertiesInput");
    m_heightPS = new QLineEdit("0.0"); m_heightPS->setValidator(validator); m_heightPS->setObjectName("PropertiesInput");
    l2->addWidget(new QLabel("X:"), 0, 0);
    l2->addWidget(m_pointX, 0, 1);
    l2->addWidget(new QLabel("Y:"), 0, 2);
    l2->addWidget(m_pointY, 0, 3);
    l2->addWidget(new QLabel("W:"), 0, 4);
    l2->addWidget(m_widthPS, 0, 5);
    l2->addWidget(new QLabel("H:"), 0, 6);
    l2->addWidget(m_heightPS, 0, 7);
    m_modeStack->addWidget(pagePointSize);

    layout->addWidget(m_modeStack, 1, 0, 1, 8);

    //угол поворота и скругление (общие для всех режимов)
    m_rotation = new QLineEdit("0.0"); m_rotation->setValidator(validator); m_rotation->setObjectName("PropertiesInput");
    m_cornerTypeCombo = new QComboBox();
    m_cornerTypeCombo->addItem("Без скр.", static_cast<int>(CornerType::None));
    m_cornerTypeCombo->addItem("Скругл.", static_cast<int>(CornerType::Fillet));
    m_cornerTypeCombo->addItem("Фаска", static_cast<int>(CornerType::Chamfer));
    m_cornerTypeCombo->setObjectName("PropertiesComboBox");
    m_cornerRadiusEdit = new QLineEdit("0.0");
    m_cornerRadiusEdit->setValidator(validator);
    m_cornerRadiusEdit->setObjectName("PropertiesInput");

    layout->addWidget(new QLabel("Угол:"), 2, 0);
    layout->addWidget(m_rotation, 2, 1);
    layout->addWidget(new QLabel("Углы:"), 2, 2);
    layout->addWidget(m_cornerTypeCombo, 2, 3);
    layout->addWidget(new QLabel("R:"), 2, 4);
    layout->addWidget(m_cornerRadiusEdit, 2, 5);

    //полярные координаты (минимальные)
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
    m_modeStack->setCurrentIndex(index);
}

void RectanglePropertiesWidget::setPrimitives(const QList<BasePrimitive*>& primitives)
{
    BasePropertiesWidget::setPrimitives(primitives);
    m_currentRect = nullptr;
    if(m_currentPrimitive && m_currentPrimitive->getType() == PrimitiveType::Rectangle) {
        m_currentRect = static_cast<RectanglePrimitive*>(m_currentPrimitive);
    }
    m_modeComboBox->setCurrentIndex(1); // По умолчанию "Центр+размер" для редактирования
    updateFieldValues();
}

void RectanglePropertiesWidget::updateFieldValues()
{
    if(m_currentRect) {
        PointPrimitive center = m_currentRect->getCenter();
        double w = m_currentRect->getWidth();
        double h = m_currentRect->getHeight();

        //Страница 0: Две точки (вычисляем углы из центра)
        double x1 = center.getX() - w / 2.0;
        double y1 = center.getY() - h / 2.0;
        double x2 = center.getX() + w / 2.0;
        double y2 = center.getY() + h / 2.0;
        m_p1X->setText(QString::number(x1, 'f', 2));
        m_p1Y->setText(QString::number(y1, 'f', 2));
        m_p2X->setText(QString::number(x2, 'f', 2));
        m_p2Y->setText(QString::number(y2, 'f', 2));

        //Страница 1: Центр + размер
        m_centerX->setText(QString::number(center.getX(), 'f', 2));
        m_centerY->setText(QString::number(center.getY(), 'f', 2));
        m_width->setText(QString::number(w, 'f', 2));
        m_height->setText(QString::number(h, 'f', 2));

        //Страница 2: Точка + размер (угол = левый нижний)
        m_pointX->setText(QString::number(x1, 'f', 2));
        m_pointY->setText(QString::number(y1, 'f', 2));
        m_widthPS->setText(QString::number(w, 'f', 2));
        m_heightPS->setText(QString::number(h, 'f', 2));

        //Общие
        m_rotation->setText(QString::number(m_currentRect->getRotation(), 'f', 2));
        m_centerRadius->setText(QString::number(center.getRadius(), 'f', 2));
        m_centerAngle->setText(QString::number(center.getAngle(), 'f', 2));

        int cornerIdx = m_cornerTypeCombo->findData(static_cast<int>(m_currentRect->getCornerType()));
        if (cornerIdx >= 0) m_cornerTypeCombo->setCurrentIndex(cornerIdx);
        m_cornerRadiusEdit->setText(QString::number(m_currentRect->getCornerRadius(), 'f', 2));
    } else {
        m_p1X->setText("0.0"); m_p1Y->setText("0.0");
        m_p2X->setText("0.0"); m_p2Y->setText("0.0");
        m_centerX->setText("0.0"); m_centerY->setText("0.0");
        m_width->setText("0.0"); m_height->setText("0.0");
        m_pointX->setText("0.0"); m_pointY->setText("0.0");
        m_widthPS->setText("0.0"); m_heightPS->setText("0.0");
        m_rotation->setText("0.0");
        m_centerRadius->setText("0.0"); m_centerAngle->setText("0.0");
        m_cornerTypeCombo->setCurrentIndex(0);
        m_cornerRadiusEdit->setText("0.0");
    }
}

void RectanglePropertiesWidget::onApplyButtonClicked()
{
    PointPrimitive center;
    double w = 0, h = 0;

    int mode = m_modeComboBox->currentIndex();

    if (mode == 0) {
        // Две точки
        double x1 = m_p1X->text().toDouble();
        double y1 = m_p1Y->text().toDouble();
        double x2 = m_p2X->text().toDouble();
        double y2 = m_p2Y->text().toDouble();
        center.setX((x1 + x2) / 2.0);
        center.setY((y1 + y2) / 2.0);
        w = std::abs(x2 - x1);
        h = std::abs(y2 - y1);
    }
    else if (mode == 1) {
        // Центр + размер
        if (m_selectedCoordSystem == CoordinateSystemType::Cartesian) {
            center.setX(m_centerX->text().toDouble());
            center.setY(m_centerY->text().toDouble());
        } else {
            center.setPolar(m_centerRadius->text().toDouble(), m_centerAngle->text().toDouble());
        }
        w = m_width->text().toDouble();
        h = m_height->text().toDouble();
    }
    else if (mode == 2) {
        // Точка + размер
        double px = m_pointX->text().toDouble();
        double py = m_pointY->text().toDouble();
        w = m_widthPS->text().toDouble();
        h = m_heightPS->text().toDouble();
        center.setX(px + w / 2.0);
        center.setY(py + h / 2.0);
    }

    double r = m_rotation->text().toDouble();
    CornerType cornerType = static_cast<CornerType>(m_cornerTypeCombo->currentData().toInt());
    double cornerRadius = m_cornerRadiusEdit->text().toDouble();

    int typeEmit = m_selectedLineTypeId;
    if(typeEmit == -1 && m_currentRect) typeEmit = m_currentRect->getLineType();
    if(typeEmit == -1) typeEmit = (int)LineType::SolidMain;

    emit propertiesApplied(m_currentRect, center, w, h, r, cornerType, cornerRadius, 
                           m_selectedColor, (LineType)typeEmit);
}
