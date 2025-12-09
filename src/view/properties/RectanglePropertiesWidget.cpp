#include "RectanglePropertiesWidget.h"
#include "RectanglePrimitive.h"
#include <QFormLayout>
#include <QDoubleValidator>
#include <QPushButton>
#include <QtMath>

RectanglePropertiesWidget::RectanglePropertiesWidget(QWidget* parent) : BasePropertiesWidget(parent)
{
    auto* validator = new QDoubleValidator(this);

    // --- Декартовы координаты ---
    auto* cartesianLayout = static_cast<QFormLayout*>(m_cartesianWidgets->layout());
    m_centerX = new QLineEdit("0.0"); m_centerX->setValidator(validator);
    m_centerY = new QLineEdit("0.0"); m_centerY->setValidator(validator);

    cartesianLayout->addRow("Центр X:", m_centerX);
    cartesianLayout->addRow("Центр Y:", m_centerY);

    // --- Полярные координаты ---
    auto* polarLayout = static_cast<QFormLayout*>(m_polarWidgets->layout());
    m_centerRadius = new QLineEdit("0.0"); m_centerRadius->setValidator(validator);
    m_centerAngle  = new QLineEdit("0.0"); m_centerAngle->setValidator(validator);

    polarLayout->addRow("Центр R:", m_centerRadius);
    polarLayout->addRow("Центр A:", m_centerAngle);

    // --- Общие параметры (добавляем в оба лайаута или в общий блок) ---
    // Так как BasePropertiesWidget разделяет стеком cartesian/polar, добавим общие поля в центральную колонку ПОД стеком.
    // Но архитектура BasePropertiesWidget предполагает стек внутри.
    // Дублируем поля визуально или добавляем их в оба лайаута (нельзя один виджет в два лайаута).
    // Решение: Создаем независимые поля для каждого лайаута или выносим общие параметры из стека.
    // Для простоты сейчас добавим их в оба лайаута (создав дубликаты указателей нельзя).
    // Сделаем проще: Добавим их в m_cartesianWidgets и m_polarWidgets "вручную" каждый раз.

    // Чтобы не усложнять BasePropertiesWidget, добавим общие поля в ОБА лайаута.
    // Но QWidget не может быть в двух местах.
    // Поэтому создаем поля один раз, а добавляем их в layout'ы "по ситуации"? Нет.
    // Правильнее: Добавить их в m_centralColumn (QVBoxLayout) ПОСЛЕ m_paramsStack.

    auto* centralLayout = static_cast<QVBoxLayout*>(m_centralColumn->layout());

    // Создаем контейнер для общих свойств
    QWidget* commonWidget = new QWidget();
    auto* commonLayout = new QFormLayout(commonWidget);
    commonLayout->setContentsMargins(0, 10, 10, 0); // Отступ сверху

    m_width   = new QLineEdit("0.0"); m_width->setValidator(validator);
    m_height  = new QLineEdit("0.0"); m_height->setValidator(validator);
    m_rotation= new QLineEdit("0.0"); m_rotation->setValidator(validator);

    commonLayout->addRow("Ширина:", m_width);
    commonLayout->addRow("Высота:", m_height);
    commonLayout->addRow("Поворот (°):", m_rotation);

    // Добавляем этот виджет в центральную колонку под стеком координат
    // m_paramsStack добавлен в конструкторе BasePropertiesWidget.
    // Мы добавляем commonWidget следом.
    centralLayout->addWidget(commonWidget);

    connect(m_applyButton, &QPushButton::clicked, this, &RectanglePropertiesWidget::onApplyButtonClicked);
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
    } else {
        // Дефолт для создания
        m_centerX->setText("0.0"); m_centerY->setText("0.0");
        m_centerRadius->setText("0.0"); m_centerAngle->setText("0.0");
        m_width->setText("0.0"); m_height->setText("0.0"); m_rotation->setText("0.0");
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

    int typeEmit = m_selectedLineTypeId;
    if(typeEmit == -1 && m_currentRect) typeEmit = m_currentRect->getLineType();
    if(typeEmit == -1) typeEmit = (int)LineType::SolidMain;

    emit propertiesApplied(m_currentRect, center, w, h, r, m_selectedColor, (LineType)typeEmit);
}
