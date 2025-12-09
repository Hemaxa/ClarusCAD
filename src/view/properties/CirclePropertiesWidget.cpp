#include "CirclePropertiesWidget.h"
#include "CirclePrimitive.h"
#include "ThemeManager.h"

#include <QFormLayout>
#include <QDoubleValidator>
#include <QPushButton>
#include <QComboBox>
#include <QtMath>
#include <QMessageBox>

CirclePropertiesWidget::CirclePropertiesWidget(QWidget* parent) : BasePropertiesWidget(parent)
{
    auto* validator = new QDoubleValidator(this);

    // Добавляем выбор режима в начало правого layout'а (или центрального)
    // В BasePropertiesWidget layout'ы уже созданы. Добавим в m_cartesianWidgets layout
    auto* mainLayout = static_cast<QFormLayout*>(m_cartesianWidgets->layout());

    m_modeComboBox = new QComboBox();
    m_modeComboBox->addItem("Центр и Радиус");
    m_modeComboBox->addItem("Центр и Диаметр");
    m_modeComboBox->addItem("Две точки (Диаметр)");
    m_modeComboBox->addItem("Три точки");

    connect(m_modeComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &CirclePropertiesWidget::onModeChanged);

    mainLayout->addRow("Способ задания:", m_modeComboBox);

    // Создаем StackedWidget для смены полей
    m_modeStack = new QStackedWidget();

    // --- СТРАНИЦА 1: Центр + Радиус ---
    m_pageCenterRadius = new QWidget();
    auto* l1 = new QFormLayout(m_pageCenterRadius);
    l1->setContentsMargins(0,0,0,0);
    m_crCenterX = new QLineEdit("0.0"); m_crCenterX->setValidator(validator);
    m_crCenterY = new QLineEdit("0.0"); m_crCenterY->setValidator(validator);
    m_crRadius  = new QLineEdit("0.0"); m_crRadius->setValidator(validator);
    l1->addRow("Центр X:", m_crCenterX);
    l1->addRow("Центр Y:", m_crCenterY);
    l1->addRow("Радиус:", m_crRadius);
    m_modeStack->addWidget(m_pageCenterRadius);

    // --- СТРАНИЦА 2: Центр + Диаметр ---
    m_pageCenterDiameter = new QWidget();
    auto* l2 = new QFormLayout(m_pageCenterDiameter);
    l2->setContentsMargins(0,0,0,0);
    m_cdCenterX = new QLineEdit("0.0"); m_cdCenterX->setValidator(validator);
    m_cdCenterY = new QLineEdit("0.0"); m_cdCenterY->setValidator(validator);
    m_cdDiameter = new QLineEdit("0.0"); m_cdDiameter->setValidator(validator);
    l2->addRow("Центр X:", m_cdCenterX);
    l2->addRow("Центр Y:", m_cdCenterY);
    l2->addRow("Диаметр:", m_cdDiameter);
    m_modeStack->addWidget(m_pageCenterDiameter);

    // --- СТРАНИЦА 3: Две точки ---
    m_pageTwoPoints = new QWidget();
    auto* l3 = new QFormLayout(m_pageTwoPoints);
    l3->setContentsMargins(0,0,0,0);
    m_tpX1 = new QLineEdit("0.0"); m_tpX1->setValidator(validator);
    m_tpY1 = new QLineEdit("0.0"); m_tpY1->setValidator(validator);
    m_tpX2 = new QLineEdit("0.0"); m_tpX2->setValidator(validator);
    m_tpY2 = new QLineEdit("0.0"); m_tpY2->setValidator(validator);
    l3->addRow("Точка 1 X:", m_tpX1);
    l3->addRow("Точка 1 Y:", m_tpY1);
    l3->addRow("Точка 2 X:", m_tpX2);
    l3->addRow("Точка 2 Y:", m_tpY2);
    m_modeStack->addWidget(m_pageTwoPoints);

    // --- СТРАНИЦА 4: Три точки ---
    m_pageThreePoints = new QWidget();
    auto* l4 = new QFormLayout(m_pageThreePoints);
    l4->setContentsMargins(0,0,0,0);
    m_thpX1 = new QLineEdit("0.0"); m_thpX1->setValidator(validator);
    m_thpY1 = new QLineEdit("0.0"); m_thpY1->setValidator(validator);
    m_thpX2 = new QLineEdit("0.0"); m_thpX2->setValidator(validator);
    m_thpY2 = new QLineEdit("0.0"); m_thpY2->setValidator(validator);
    m_thpX3 = new QLineEdit("0.0"); m_thpX3->setValidator(validator);
    m_thpY3 = new QLineEdit("0.0"); m_thpY3->setValidator(validator);
    l4->addRow("Точка 1 X:", m_thpX1);
    l4->addRow("Точка 1 Y:", m_thpY1);
    l4->addRow("Точка 2 X:", m_thpX2);
    l4->addRow("Точка 2 Y:", m_thpY2);
    l4->addRow("Точка 3 X:", m_thpX3);
    l4->addRow("Точка 3 Y:", m_thpY3);
    m_modeStack->addWidget(m_pageThreePoints);

    mainLayout->addRow(m_modeStack);

    connect(m_applyButton, &QPushButton::clicked, this, &CirclePropertiesWidget::onApplyButtonClicked);

    // Инициализация
    setCoordinateSystem(CoordinateSystemType::Cartesian);
}

void CirclePropertiesWidget::setPrimitives(const QList<BasePrimitive*>& primitives)
{
    BasePropertiesWidget::setPrimitives(primitives);

    m_currentCircle = nullptr;
    if (m_currentPrimitive && m_currentPrimitive->getType() == PrimitiveType::Circle) {
        m_currentCircle = static_cast<CirclePrimitive*>(m_currentPrimitive);
    }

    // При выборе новой окружности сбрасываем режим на "Центр+Радиус",
    // т.к. мы не знаем исходные точки построения (они не хранятся в модели)
    m_modeComboBox->setCurrentIndex(0);

    updateFieldValues();
}

void CirclePropertiesWidget::onModeChanged(int index)
{
    m_modeStack->setCurrentIndex(index);
    // При смене режима можно было бы пытаться пересчитать координаты (например, из центра сделать фиктивные точки),
    // но это может запутать. Оставим 0.0 или текущие значения.
}

void CirclePropertiesWidget::updateFieldValues()
{
    // Пока поддерживаем только Декартову систему в этом виджете полностью
    // Если нужна полярная - можно добавить проверку.

    if (m_currentCircle) {
        PointPrimitive center = m_currentCircle->getCenter();
        double r = m_currentCircle->getRadius();

        // Заполняем только первый режим (стандартный), остальные - по нулям или фиктивные
        m_crCenterX->setText(QString::number(center.getX()));
        m_crCenterY->setText(QString::number(center.getY()));
        m_crRadius->setText(QString::number(r));

        m_cdCenterX->setText(QString::number(center.getX()));
        m_cdCenterY->setText(QString::number(center.getY()));
        m_cdDiameter->setText(QString::number(r * 2.0));

        // Для 2 и 3 точек мы не можем восстановить исходные данные,
        // поэтому оставляем поля пустыми или 0, предлагая пользователю ВВЕСТИ новые
        // если он хочет изменить окружность этим способом.
    } else {
        // Дефолтные значения
        m_crCenterX->setText("0.0"); m_crCenterY->setText("0.0"); m_crRadius->setText("0.0");
    }
}

// Заглушки для старых слотов
void CirclePropertiesWidget::onCartesianRadiusChanged(const QString& text) {}
void CirclePropertiesWidget::onCartesianDiameterChanged(const QString& text) {}

void CirclePropertiesWidget::onApplyButtonClicked()
{
    PointPrimitive center;
    double radius = 0.0;

    int mode = m_modeComboBox->currentIndex();
    bool isValid = true;

    if (mode == 0) { // Center + Radius
        center.setX(m_crCenterX->text().toDouble());
        center.setY(m_crCenterY->text().toDouble());
        radius = m_crRadius->text().toDouble();
    }
    else if (mode == 1) { // Center + Diameter
        center.setX(m_cdCenterX->text().toDouble());
        center.setY(m_cdCenterY->text().toDouble());
        radius = m_cdDiameter->text().toDouble() / 2.0;
    }
    else if (mode == 2) { // 2 Points
        double x1 = m_tpX1->text().toDouble();
        double y1 = m_tpY1->text().toDouble();
        double x2 = m_tpX2->text().toDouble();
        double y2 = m_tpY2->text().toDouble();

        center.setX((x1 + x2) / 2.0);
        center.setY((y1 + y2) / 2.0);
        radius = std::sqrt(std::pow(x2 - x1, 2) + std::pow(y2 - y1, 2)) / 2.0;
    }
    else if (mode == 3) { // 3 Points
        PointPrimitive p1(m_thpX1->text().toDouble(), m_thpY1->text().toDouble());
        PointPrimitive p2(m_thpX2->text().toDouble(), m_thpY2->text().toDouble());
        PointPrimitive p3(m_thpX3->text().toDouble(), m_thpY3->text().toDouble());

        if (!getCircleFrom3Points(p1, p2, p3, center, radius)) {
            QMessageBox::warning(this, "Ошибка", "Невозможно построить окружность по этим 3 точкам (лежат на одной прямой).");
            isValid = false;
        }
    }

    if (isValid && radius >= 0) {
        int typeToEmit = m_selectedLineTypeId;
        if(typeToEmit == -1 && m_currentCircle) typeToEmit = m_currentCircle->getLineType();
        if(typeToEmit == -1) typeToEmit = (int)LineType::SolidMain;

        emit propertiesApplied(m_currentCircle, center, radius, m_selectedColor, static_cast<LineType>(typeToEmit));

        // После применения можно обновить поля (например, переключить на Center+Radius и показать результат)
        // Но оставим как есть, чтобы пользователь мог корректировать ввод.
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

void CirclePropertiesWidget::updatePrompt()
{
    ThemeManager& tm = ThemeManager::instance();
    QMap<QString, QColor> colorMap;
    colorMap.insert("currentColor", tm.getIconColor());
    QPixmap pix = tm.colorizeSvg(":/icons/icons/tools/circle.svg", colorMap);
    if (!pix.isNull()) {
        m_leftColumn->setPixmap(pix.scaledToWidth(40, Qt::SmoothTransformation));
    }
}
