#include "SegmentPropertiesWidget.h"
#include "SegmentCreationPrimitive.h"

#include <QFormLayout>
#include <QLineEdit>
#include <QDoubleValidator>
#include <QPushButton>

SegmentPropertiesWidget::SegmentPropertiesWidget(QWidget* parent) : BasePropertiesWidget(parent)
{
    auto* layout = new QFormLayout(this);
    m_startXEdit = new QLineEdit("0.0");
    m_startYEdit = new QLineEdit("0.0");
    m_endXEdit = new QLineEdit("0.0");
    m_endYEdit = new QLineEdit("0.0");

    auto* validator = new QDoubleValidator(this);
    m_startXEdit->setValidator(validator);
    m_startYEdit->setValidator(validator);
    m_endXEdit->setValidator(validator);
    m_endYEdit->setValidator(validator);

    layout->addRow("Начало X:", m_startXEdit);
    layout->addRow("Начало Y:", m_startYEdit);
    layout->addRow("Конец X:", m_endXEdit);
    layout->addRow("Конец Y:", m_endYEdit);

    auto* createButton = new QPushButton("Создать отрезок", this);
    layout->addWidget(createButton);
    connect(createButton, &QPushButton::clicked, this, &SegmentPropertiesWidget::onCreateButtonClicked);
}

void SegmentPropertiesWidget::setPrimitive(BasePrimitive* primitive)
{
    m_currentSegment = dynamic_cast<SegmentCreationPrimitive*>(primitive);
    if (m_currentSegment) {
        m_startXEdit->setText(QString::number(m_currentSegment->getStart().x()));
        m_startYEdit->setText(QString::number(m_currentSegment->getStart().y()));
        m_endXEdit->setText(QString::number(m_currentSegment->getEnd().x()));
        m_endYEdit->setText(QString::number(m_currentSegment->getEnd().y()));
    }
}

void SegmentPropertiesWidget::onCreateButtonClicked()
{
    // Считываем значения из полей
    PointCreationPrimitive start(m_startXEdit->text().toDouble(), m_startYEdit->text().toDouble());
    PointCreationPrimitive end(m_endXEdit->text().toDouble(), m_endYEdit->text().toDouble());

    // Отправляем сигнал!
    emit createSegmentRequested(start, end);
}
