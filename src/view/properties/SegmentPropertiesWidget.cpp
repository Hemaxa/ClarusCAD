#include "SegmentPropertiesWidget.h"
#include "SegmentPrimitive.h"

#include <QFormLayout>
#include <QLineEdit>
#include <QDoubleValidator> //класс проверки типа вводимых значений (Double)
#include <QPushButton>

SegmentPropertiesWidget::SegmentPropertiesWidget(QWidget* parent) : BasePropertiesWidget(parent)
{
    auto* validator = new QDoubleValidator(this);

    // 1. Создаем уникальные для отрезка поля
    m_startXEdit = new QLineEdit("0.0");
    m_startYEdit = new QLineEdit("0.0");
    m_endXEdit = new QLineEdit("0.0");
    m_endYEdit = new QLineEdit("0.0");

    m_startXEdit->setValidator(validator);
    m_startYEdit->setValidator(validator);
    m_endXEdit->setValidator(validator);
    m_endYEdit->setValidator(validator);

    // 2. Добавляем их в компоновку, унаследованную от базового класса
    m_layout->addRow("Начало X:", m_startXEdit);
    m_layout->addRow("Начало Y:", m_startYEdit);
    m_layout->addRow("Конец X:", m_endXEdit);
    m_layout->addRow("Конец Y:", m_endYEdit);
    m_layout->addRow("Цвет:", m_colorButton);    // <-- Добавляем кнопку цвета из базового класса
    m_layout->addWidget(m_applyButton);         // <-- Добавляем кнопку "Применить" из базового класса

    // 3. Подключаемся к кнопке из базового класса
    connect(m_applyButton, &QPushButton::clicked, this, &SegmentPropertiesWidget::onApplyButtonClicked);
}

void SegmentPropertiesWidget::setPrimitive(BasePrimitive* primitive)
{
    // 1. Вызываем базовую реализацию, чтобы настроить кнопки и цвета
    BasePropertiesWidget::setPrimitive(primitive);

    // 2. Выполняем свою специфическую логику
    m_currentSegment = dynamic_cast<SegmentPrimitive*>(primitive);

    if (m_currentSegment) {
        // Заполняем поля, если редактируем существующий объект
        m_startXEdit->setText(QString::number(m_currentSegment->getStart().getX()));
        m_startYEdit->setText(QString::number(m_currentSegment->getStart().getY()));
        m_endXEdit->setText(QString::number(m_currentSegment->getEnd().getX()));
        m_endYEdit->setText(QString::number(m_currentSegment->getEnd().getY()));
    } else {
        // Сбрасываем поля, если создаем новый
        m_startXEdit->setText("0.0");
        m_startYEdit->setText("0.0");
        m_endXEdit->setText("0.0");
        m_endYEdit->setText("0.0");
    }
}

void SegmentPropertiesWidget::onApplyButtonClicked()
{
    PointPrimitive start(m_startXEdit->text().toDouble(), m_startYEdit->text().toDouble());
    PointPrimitive end(m_endXEdit->text().toDouble(), m_endYEdit->text().toDouble());

    // Отправляем сигнал, используя m_selectedColor из базового класса
    emit propertiesApplied(m_currentSegment, start, end, m_selectedColor);
}
