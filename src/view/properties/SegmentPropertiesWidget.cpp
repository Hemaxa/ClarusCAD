#include "SegmentPropertiesWidget.h"
#include "SegmentPrimitive.h"
#include "ThemeManager.h"

#include <QFormLayout>
#include <QStackedWidget>
#include <QLineEdit>
#include <QDoubleValidator>
#include <QPushButton>

SegmentPropertiesWidget::SegmentPropertiesWidget(QWidget* parent) : BasePropertiesWidget(parent)
{
    auto* validator = new QDoubleValidator(this);

    //заполнение панели декартовых координат
    auto* cartesianLayout = static_cast<QFormLayout*>(m_cartesianWidgets->layout());
    m_startXEdit = new QLineEdit("0.0");
    m_startYEdit = new QLineEdit("0.0");
    m_endXEdit = new QLineEdit("0.0");
    m_endYEdit = new QLineEdit("0.0");
    m_startXEdit->setValidator(validator);
    m_startYEdit->setValidator(validator);
    m_endXEdit->setValidator(validator);
    m_endYEdit->setValidator(validator);
    cartesianLayout->addRow("Первая точка X:", m_startXEdit);
    cartesianLayout->addRow("Первая точка Y:", m_startYEdit);
    cartesianLayout->addRow("Вторая точка X:", m_endXEdit);
    cartesianLayout->addRow("Вторая точка Y:", m_endYEdit);

    //заполнение панели полярных координат
    auto* polarLayout = static_cast<QFormLayout*>(m_polarWidgets->layout());
    m_startRadiusEdit = new QLineEdit("0.0");
    m_startAngleEdit = new QLineEdit("0.0");
    m_endRadiusEdit = new QLineEdit("0.0");
    m_endAngleEdit = new QLineEdit("0.0");
    m_startRadiusEdit->setValidator(validator);
    m_startAngleEdit->setValidator(validator);
    m_endRadiusEdit->setValidator(validator);
    m_endAngleEdit->setValidator(validator);
    polarLayout->addRow("Первая точка R:", m_startRadiusEdit);
    polarLayout->addRow("Первая точка A:", m_startAngleEdit);
    polarLayout->addRow("Вторая точка R:", m_endRadiusEdit);
    polarLayout->addRow("Вторая точка A:", m_endAngleEdit);

    //подключение сигнала от кнопки
    connect(m_applyButton, &QPushButton::clicked, this, &SegmentPropertiesWidget::onApplyButtonClicked);

    // 4. Устанавливаем начальные значения
    setCoordinateSystem(CoordinateSystemType::Cartesian);
}

void SegmentPropertiesWidget::setPrimitive(BasePrimitive* primitive)
{
    BasePropertiesWidget::setPrimitive(primitive);
    m_currentSegment = dynamic_cast<SegmentPrimitive*>(primitive);
    updateFieldValues();
}

void SegmentPropertiesWidget::updateFieldValues()
{
    bool isCartesian = (m_coordSystem == CoordinateSystemType::Cartesian);

    if (m_currentSegment) {
        const auto& startPoint = m_currentSegment->getStart();
        const auto& endPoint = m_currentSegment->getEnd();

        if (isCartesian) {
            m_startXEdit->setText(QString::number(startPoint.getX()));
            m_startYEdit->setText(QString::number(startPoint.getY()));
            m_endXEdit->setText(QString::number(endPoint.getX()));
            m_endYEdit->setText(QString::number(endPoint.getY()));
        } else {
            m_startRadiusEdit->setText(QString::number(startPoint.getRadius()));
            m_startAngleEdit->setText(QString::number(startPoint.getAngle()));
            m_endRadiusEdit->setText(QString::number(endPoint.getRadius()));
            m_endAngleEdit->setText(QString::number(endPoint.getAngle()));
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

void SegmentPropertiesWidget::updatePrompt()
{
    ThemeManager& tm = ThemeManager::instance();

    QMap<QString, QColor> colorMap;
    colorMap.insert("currentColor", tm.getIconColor());
    colorMap.insert("@textColor", tm.getColor("textColor"));

    QPixmap themedPixmap = tm.colorizeSvg(":/promts/promts/segment-promt.svg", colorMap);

    m_leftColumn->setPixmap(themedPixmap);
}


void SegmentPropertiesWidget::onApplyButtonClicked()
{
    PointPrimitive start, end;
    bool isCartesian = (m_coordSystem == CoordinateSystemType::Cartesian);

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

    emit propertiesApplied(m_currentSegment, start, end, m_selectedColor);
}
