#include "LinePropertiesWidget.h"
#include "Point.h"

#include <QLineEdit>
#include <QPushButton>
#include <QFormLayout>
#include <QDoubleValidator>

LinePropertiesWidget::LinePropertiesWidget(QWidget* parent) : QWidget(parent)
{
    auto* layout = new QFormLayout(this);

    m_startXEdit = new QLineEdit("10.0");
    m_startYEdit = new QLineEdit("10.0");
    m_endXEdit = new QLineEdit("300.0");
    m_endYEdit = new QLineEdit("250.0");

    // Валидаторы, чтобы пользователь мог вводить только числа
    auto* validator = new QDoubleValidator(this);
    m_startXEdit->setValidator(validator);
    m_startYEdit->setValidator(validator);
    m_endXEdit->setValidator(validator);
    m_endYEdit->setValidator(validator);

    layout->addRow("Начало X:", m_startXEdit);
    layout->addRow("Начало Y:", m_startYEdit);
    layout->addRow("Конец X:", m_endXEdit);
    layout->addRow("Конец Y:", m_endYEdit);

    auto* createButton = new QPushButton("Создать отрезок");
    layout->addWidget(createButton);

    connect(createButton, &QPushButton::clicked, this, &LinePropertiesWidget::onCreateButtonClicked);
}

void LinePropertiesWidget::onCreateButtonClicked()
{
    Point start(m_startXEdit->text().toDouble(), m_startYEdit->text().toDouble());
    Point end(m_endXEdit->text().toDouble(), m_endYEdit->text().toDouble());
    emit createSegmentRequested(start, end);
}
