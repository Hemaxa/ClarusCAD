#include "SegmentPropertiesWidget.h"
#include "SegmentPrimitive.h"

#include <QFormLayout>
#include <QLineEdit>
#include <QDoubleValidator> //класс проверки типа вводимых значений (Double)
#include <QPushButton>

SegmentPropertiesWidget::SegmentPropertiesWidget(QWidget* parent) : BasePropertiesWidget(parent)
{
    //формовый шаблон компоновки (автоматически создает два столбца, для меток и полей ввода)
    auto* layout = new QFormLayout(this);

    //создание полей ввода
    m_startXEdit = new QLineEdit("0.0");
    m_startYEdit = new QLineEdit("0.0");
    m_endXEdit = new QLineEdit("0.0");
    m_endYEdit = new QLineEdit("0.0");

    //прикрепление валидатора к полям ввода
    auto* validator = new QDoubleValidator(this);
    m_startXEdit->setValidator(validator);
    m_startYEdit->setValidator(validator);
    m_endXEdit->setValidator(validator);
    m_endYEdit->setValidator(validator);

    //создание меток
    layout->addRow("Начало X:", m_startXEdit);
    layout->addRow("Начало Y:", m_startYEdit);
    layout->addRow("Конец X:", m_endXEdit);
    layout->addRow("Конец Y:", m_endYEdit);

    //создание кнопки
    auto* createButton = new QPushButton("Создать отрезок", this);
    layout->addWidget(createButton);
    connect(createButton, &QPushButton::clicked, this, &SegmentPropertiesWidget::onCreateButtonClicked);
}

void SegmentPropertiesWidget::setPrimitive(BasePrimitive* primitive)
{
    //указатель, переданный из PropertiesPanelWidget преобразуется в указатель, используемый в SegmentCreationPrimitive
    m_currentSegment = dynamic_cast<SegmentPrimitive*>(primitive);
    //если такой указатель есть, значит существует такой объект, значит надо показать его параметры
    if (m_currentSegment) {
        m_startXEdit->setText(QString::number(m_currentSegment->getStart().getX()));
        m_startYEdit->setText(QString::number(m_currentSegment->getStart().getY()));
        m_endXEdit->setText(QString::number(m_currentSegment->getEnd().getX()));
        m_endYEdit->setText(QString::number(m_currentSegment->getEnd().getY()));
    }
    //если такого указателя нет, значит создается новый объект, значит надо показать пустые параметры
    else {
        m_startXEdit->setText("0.0");
        m_startYEdit->setText("0.0");
        m_endXEdit->setText("0.0");
        m_endYEdit->setText("0.0");
    }
}

void SegmentPropertiesWidget::onCreateButtonClicked()
{
    //считывается значение полей
    PointPrimitive start(m_startXEdit->text().toDouble(), m_startYEdit->text().toDouble());
    PointPrimitive end(m_endXEdit->text().toDouble(), m_endYEdit->text().toDouble());

    //отправка сигнала
    emit createSegmentRequested(start, end);
}
