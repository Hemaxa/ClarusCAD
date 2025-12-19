#include "SplinePropertiesWidget.h"
#include "SplinePrimitive.h"
#include "ThemeManager.h"

#include <QFormLayout>
#include <QCheckBox>
#include <QPushButton>
#include <QLineEdit>
#include <QHBoxLayout>
#include <QDoubleValidator>
#include <QLabel>

SplinePropertiesWidget::SplinePropertiesWidget(QWidget* parent) : BasePropertiesWidget(parent)
{
    auto* validator = new QDoubleValidator(this);
    
    //заполнение панели декартовых координат (используем для параметров сплайна)
    auto* cartesianLayout = static_cast<QFormLayout*>(m_cartesianWidgets->layout());
    
    //замкнутый/разомкнутый
    m_closedCheckBox = new QCheckBox("Замкнутый сплайн");
    m_closedCheckBox->setChecked(false);
    cartesianLayout->addRow("", m_closedCheckBox);

    //подключение сигнала для обновления инструмента в реальном времени
    connect(m_closedCheckBox, &QCheckBox::toggled, this, &SplinePropertiesWidget::onClosedChanged);

    //создание прокручиваемой области для контрольных точек
    m_pointsScrollArea = new QScrollArea();
    m_pointsScrollArea->setWidgetResizable(true);
    m_pointsScrollArea->setMinimumHeight(180);
    m_pointsScrollArea->setMaximumHeight(250);
    m_pointsScrollArea->setMinimumWidth(250);
    m_pointsScrollArea->setFrameShape(QFrame::NoFrame);
    
    m_pointsContainer = new QWidget();
    m_pointsLayout = new QVBoxLayout(m_pointsContainer);
    m_pointsLayout->setContentsMargins(0, 0, 0, 0);
    m_pointsLayout->setSpacing(4);
    m_pointsLayout->addStretch();
    
    m_pointsScrollArea->setWidget(m_pointsContainer);
    
    //добавляем в центральную колонку
    auto* centralLayout = static_cast<QVBoxLayout*>(m_centralColumn->layout());
    
    QLabel* pointsLabel = new QLabel("Контрольные точки:");
    centralLayout->addWidget(pointsLabel);
    centralLayout->addWidget(m_pointsScrollArea);

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
    //если существующий объект
    if (m_currentSpline) {
        m_closedCheckBox->setChecked(m_currentSpline->isClosed());
        rebuildControlPointsUI();
    } else {
        //очистка полей для нового объекта
        for (auto& row : m_pointRows) {
            delete row.container;
        }
        m_pointRows.clear();
    }
}

void SplinePropertiesWidget::rebuildControlPointsUI()
{
    //удаляем старые поля
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
        
        //вставляем перед stretch
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
        //удаляем поля
        delete m_pointRows[index].container;
        m_pointRows.removeAt(index);
        
        //обновляем индексы и метки
        for (int i = 0; i < m_pointRows.size(); ++i) {
            m_pointRows[i].deleteBtn->setProperty("pointIndex", i);
            //обновляем метку с номером
            QLabel* label = m_pointRows[i].container->findChild<QLabel*>();
            if (label) {
                label->setText(QString::number(i + 1) + ":");
            }
        }
        
        //автоматически применяем изменения к сплайну
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
