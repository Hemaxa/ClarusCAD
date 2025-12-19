#include "SplinePropertiesWidget.h"
#include "SplinePrimitive.h"
#include "ThemeManager.h"

#include <QGridLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QCheckBox>
#include <QPushButton>
#include <QLineEdit>
#include <QDoubleValidator>
#include <QLabel>

SplinePropertiesWidget::SplinePropertiesWidget(QWidget* parent) : BasePropertiesWidget(parent)
{
    auto* validator = new QDoubleValidator(this);
    
    //получаем grid layout декартовых координат
    auto* cartesianLayout = static_cast<QGridLayout*>(m_cartesianWidgets->layout());
    
    //замкнутый/разомкнутый
    m_closedCheckBox = new QCheckBox("Замкнутый");
    m_closedCheckBox->setChecked(false);
    cartesianLayout->addWidget(m_closedCheckBox, 0, 0, 1, 4);

    //подключение сигнала для обновления инструмента в реальном времени
    connect(m_closedCheckBox, &QCheckBox::toggled, this, &SplinePropertiesWidget::onClosedChanged);

    //создание прокручиваемой области для контрольных точек
    m_pointsScrollArea = new QScrollArea();
    m_pointsScrollArea->setWidgetResizable(true);
    m_pointsScrollArea->setMinimumHeight(120);
    m_pointsScrollArea->setMaximumHeight(180);
    m_pointsScrollArea->setFrameShape(QFrame::NoFrame);
    
    m_pointsContainer = new QWidget();
    m_pointsLayout = new QVBoxLayout(m_pointsContainer);
    m_pointsLayout->setContentsMargins(0, 0, 0, 0);
    m_pointsLayout->setSpacing(4);
    m_pointsLayout->addStretch();
    
    m_pointsScrollArea->setWidget(m_pointsContainer);
    
    //добавляем область прокрутки в layout
    cartesianLayout->addWidget(new QLabel("Точки:"), 1, 0);
    cartesianLayout->addWidget(m_pointsScrollArea, 2, 0, 1, 4);

    //подключение сигнала от кнопки
    connect(m_applyButton, &QPushButton::clicked, this, &SplinePropertiesWidget::onApplyButtonClicked);

    //установка системы координат по-умолчанию
    setCoordinateSystem(CoordinateSystemType::Cartesian);
}

void SplinePropertiesWidget::setPrimitives(const QList<BasePrimitive*>& primitives)
{
    BasePropertiesWidget::setPrimitives(primitives);

    m_currentSpline = nullptr;
    if (m_currentPrimitive) {
        m_currentSpline = dynamic_cast<SplinePrimitive*>(m_currentPrimitive);
    }

    updateFieldValues();
}

void SplinePropertiesWidget::updateFieldValues()
{
    if (m_currentSpline) {
        m_closedCheckBox->setChecked(m_currentSpline->isClosed());
        rebuildControlPointsUI();
    } else {
        for (auto& row : m_pointRows) {
            delete row.container;
        }
        m_pointRows.clear();
    }
}

void SplinePropertiesWidget::rebuildControlPointsUI()
{
    for (auto& row : m_pointRows) {
        delete row.container;
    }
    m_pointRows.clear();
    
    if (!m_currentSpline) return;
    
    QVector<QPointF> points = m_currentSpline->getControlPoints();
    auto* validator = new QDoubleValidator(this);
    
    for (int i = 0; i < points.size(); ++i) {
        PointRow row;
        
        row.container = new QWidget();
        auto* hLayout = new QHBoxLayout(row.container);
        hLayout->setContentsMargins(0, 0, 0, 0);
        hLayout->setSpacing(4);
        
        QLabel* indexLabel = new QLabel(QString::number(i + 1) + ":");
        indexLabel->setFixedWidth(20);
        
        row.xEdit = new QLineEdit(QString::number(points[i].x(), 'f', 2));
        row.xEdit->setValidator(validator);
        row.xEdit->setObjectName("PropertiesInput");
        row.xEdit->setPlaceholderText("X");
        
        row.yEdit = new QLineEdit(QString::number(points[i].y(), 'f', 2));
        row.yEdit->setValidator(validator);
        row.yEdit->setObjectName("PropertiesInput");
        row.yEdit->setPlaceholderText("Y");
        
        row.deleteBtn = new QPushButton("×");
        row.deleteBtn->setFixedSize(24, 24);
        row.deleteBtn->setToolTip("Удалить точку");
        row.deleteBtn->setProperty("pointIndex", i);
        connect(row.deleteBtn, &QPushButton::clicked, this, &SplinePropertiesWidget::onDeletePointClicked);
        
        hLayout->addWidget(indexLabel);
        hLayout->addWidget(row.xEdit);
        hLayout->addWidget(row.yEdit);
        hLayout->addWidget(row.deleteBtn);
        
        m_pointsLayout->insertWidget(m_pointsLayout->count() - 1, row.container);
        m_pointRows.append(row);
    }
}

QVector<QPointF> SplinePropertiesWidget::getControlPointsFromFields() const
{
    QVector<QPointF> points;
    for (const auto& row : m_pointRows) {
        double x = row.xEdit->text().toDouble();
        double y = row.yEdit->text().toDouble();
        points.append(QPointF(x, y));
    }
    return points;
}

void SplinePropertiesWidget::onDeletePointClicked()
{
    QPushButton* btn = qobject_cast<QPushButton*>(sender());
    if (!btn) return;
    
    int index = btn->property("pointIndex").toInt();
    
    if (index >= 0 && index < m_pointRows.size()) {
        delete m_pointRows[index].container;
        m_pointRows.removeAt(index);
        
        for (int i = 0; i < m_pointRows.size(); ++i) {
            m_pointRows[i].deleteBtn->setProperty("pointIndex", i);
            QLabel* label = m_pointRows[i].container->findChild<QLabel*>();
            if (label) {
                label->setText(QString::number(i + 1) + ":");
            }
        }
        
        if (m_currentSpline && m_pointRows.size() >= 2) {
            onApplyButtonClicked();
        }
    }
}

void SplinePropertiesWidget::onApplyButtonClicked()
{
    bool closed = m_closedCheckBox->isChecked();
    QVector<QPointF> controlPoints = getControlPointsFromFields();
    
    int typeToEmit = m_selectedLineTypeId;
    if (typeToEmit == -1 && m_currentSpline) {
        typeToEmit = m_currentSpline->getLineType();
    }
    if (typeToEmit == -1) typeToEmit = static_cast<int>(LineType::SolidMain);

    emit propertiesApplied(m_currentSpline, closed, controlPoints,
                           m_selectedColor, static_cast<LineType>(typeToEmit));
}

void SplinePropertiesWidget::onClosedChanged(bool checked)
{
    emit closedChanged(checked);
}
