#include "BasePropertiesWidget.h"
#include "BasePrimitive.h"

#include <QPushButton>
#include <QColorDialog>
#include <QFormLayout>
#include <QStackedWidget>
#include <QGridLayout>
#include <QLabel>
#include <QSpacerItem>
#include <QVBoxLayout>

BasePropertiesWidget::BasePropertiesWidget(QWidget* parent) : QWidget(parent)
{
    //установка системы координат по-умолчанию
    m_coordSystem = CoordinateSystemType::Cartesian;

    //создание главной сетки
    auto* mainLayout = new QGridLayout(this);
    mainLayout->setContentsMargins(10, 10, 10, 10);

    //1) левая колонка (подсказка)
    m_leftColumn = new QLabel();
    m_leftColumn->setAlignment(Qt::AlignTop | Qt::AlignHCenter);
    //m_leftColumn->setContentsMargins(10, 10, 10, 10);

    //2) правая колонка (общие параметры)
    m_rightColumn = new QWidget();
    auto* rightLayout = new QFormLayout(m_rightColumn);
    // rightLayout->setAlignment(Qt::AlignTop | Qt::AlignHCenter);
    // rightLayout->setContentsMargins(10, 10, 10, 10);

    //создание кнопки изменения цвета
    m_colorButton = new QPushButton();
    m_colorButton->setFixedSize(25, 25);
    m_colorButton->setCursor(Qt::PointingHandCursor);
    connect(m_colorButton, &QPushButton::clicked, this, &BasePropertiesWidget::onColorButtonClicked);
    rightLayout->addRow("Цвет:", m_colorButton);

    //3) центральная колонка (сменяемые параметры)
    m_centralColumn = new QWidget();
    auto* centralLayout = new QVBoxLayout(m_centralColumn);
    //centralLayout->setAlignment(Qt::AlignTop | Qt::AlignHCenter);
    centralLayout->setContentsMargins(0, 0, 0, 0);

    //создание и заполнение общей панели с параметрами внутри центральной колонки
    m_paramsStack = new QStackedWidget();
    m_cartesianWidgets = new QWidget();
    m_polarWidgets = new QWidget();

    new QFormLayout(m_cartesianWidgets);
    new QFormLayout(m_polarWidgets);

    m_paramsStack->addWidget(m_cartesianWidgets);
    m_paramsStack->addWidget(m_polarWidgets);
    centralLayout->addWidget(m_paramsStack);

    //создание кнопки "Создать/Обновить"
    m_applyButton = new QPushButton();
    m_applyButton->setFixedSize(120, 30);

    //добавление общей панели и кнопки в layout центральной колонки
    // centralLayout->addWidget(m_paramsStack);
    // centralLayout->addWidget(m_applyButton, 0, Qt::AlignHCenter);
    // centralLayout->addStretch(1);

    //cборка всей сетки
    mainLayout->addWidget(m_leftColumn, 0, 0, Qt::AlignHCenter);
    mainLayout->addWidget(m_centralColumn, 0, 1, Qt::AlignHCenter);
    mainLayout->addWidget(m_rightColumn, 0, 2, Qt::AlignHCenter);

    mainLayout->addWidget(m_applyButton, 1, 0, 1, 3, Qt::AlignHCenter);

    //равномерное растягивание колонок
    mainLayout->setColumnStretch(0, 1);
    mainLayout->setColumnStretch(1, 1);
    mainLayout->setColumnStretch(2, 1);

    mainLayout->setRowStretch(0, 1);
}

void BasePropertiesWidget::setPrimitive(BasePrimitive* primitive)
{
    m_currentPrimitive = primitive;

    //режим редактирования
    if (m_currentPrimitive) {
        m_selectedColor = m_currentPrimitive->getColor();
        m_applyButton->setText("Обновить");
    }
    //режим создания
    else
    {
        m_applyButton->setText("Создать");
    }

    updateColor(m_selectedColor);
    updateFieldValues();
    updatePrompt();
}

void BasePropertiesWidget::setCoordinateSystem(CoordinateSystemType type)
{
    m_coordSystem = type;

    if (m_coordSystem == CoordinateSystemType::Cartesian)
    {
        m_paramsStack->setCurrentWidget(m_cartesianWidgets);
    }
    else
    {
        m_paramsStack->setCurrentWidget(m_polarWidgets);
    }

    updateFieldValues();
    updatePrompt();
}

void BasePropertiesWidget::updateColor(const QColor& color)
{
    m_selectedColor = color;
    m_colorButton->setStyleSheet(QString("background-color: %1; border-radius: 12px; border: 1px solid gray;").arg(m_selectedColor.name()));
}

void BasePropertiesWidget::onColorButtonClicked()
{
    QColor color = QColorDialog::getColor(m_selectedColor, this, "Выберите цвет");
    if (color.isValid()) {
        m_selectedColor = color;
        updateColor(m_selectedColor);
        emit colorChanged(m_selectedColor);
    }
}

void BasePropertiesWidget::updatePrompt() {}
