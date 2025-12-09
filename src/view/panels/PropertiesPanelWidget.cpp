#include "PropertiesPanelWidget.h"

#include "SegmentPropertiesWidget.h"
#include "CirclePropertiesWidget.h"

#include "BasePrimitive.h"

#include <QStackedWidget>
#include <QVBoxLayout>

PropertiesPanelWidget::PropertiesPanelWidget(const QString& title, QWidget* parent) : BasePanelWidget(title, parent)
{
    //инициализация самой панели виджетов через QStackedWidget
    m_stack = new QStackedWidget();

    //инициализация содержимого для панели виджетов (переключаемых страниц)
    m_emptyWidget = new QWidget();
    m_segmentProperties = new SegmentPropertiesWidget();
    m_circleProperties = new CirclePropertiesWidget();
    m_rectProperties = new RectanglePropertiesWidget();
    m_arcProperties = new ArcPropertiesWidget();

    //добавление содержимого в панель
    m_stack->addWidget(m_emptyWidget);
    m_stack->addWidget(m_segmentProperties);
    m_stack->addWidget(m_circleProperties);
    m_stack->addWidget(m_rectProperties);
    m_stack->addWidget(m_arcProperties);

    auto* layout = new QVBoxLayout(canvas()); //вертикальный шаблон компоновки
    layout->setContentsMargins(0, 0, 0, 0); //убирает отступы, чтобы занять всю допустимую область панели
    layout->addWidget(m_stack); //добавление содержимого панели в canvas

    //сигналы от инструментов пересылаются (пробрасываются) в MainWindow
    //"Отрезок"
    connect(m_segmentProperties, &SegmentPropertiesWidget::propertiesApplied, this, &PropertiesPanelWidget::segmentPropertiesApplied);
    connect(m_segmentProperties, &SegmentPropertiesWidget::colorChanged, this, &PropertiesPanelWidget::colorChanged);
    connect(m_segmentProperties, &SegmentPropertiesWidget::lineTypeChanged, this, &PropertiesPanelWidget::lineTypeChanged);

    //"Окружность"
    connect(m_circleProperties, &CirclePropertiesWidget::propertiesApplied, this, &PropertiesPanelWidget::circlePropertiesApplied);
    connect(m_circleProperties, &CirclePropertiesWidget::colorChanged, this, &PropertiesPanelWidget::colorChanged);
    connect(m_circleProperties, &CirclePropertiesWidget::lineTypeChanged, this, &PropertiesPanelWidget::lineTypeChanged);

    connect(m_rectProperties, &RectanglePropertiesWidget::propertiesApplied, this, &PropertiesPanelWidget::rectanglePropertiesApplied);
    connect(m_rectProperties, &RectanglePropertiesWidget::colorChanged, this, &PropertiesPanelWidget::colorChanged);
    connect(m_rectProperties, &RectanglePropertiesWidget::lineTypeChanged, this, &PropertiesPanelWidget::lineTypeChanged);

    connect(m_arcProperties, &ArcPropertiesWidget::propertiesApplied, this, &PropertiesPanelWidget::arcPropertiesApplied);
    connect(m_arcProperties, &ArcPropertiesWidget::colorChanged, this, &PropertiesPanelWidget::colorChanged);
    connect(m_arcProperties, &ArcPropertiesWidget::lineTypeChanged, this, &PropertiesPanelWidget::lineTypeChanged);

    //минимальная высота окна
    setMinimumHeight(200);
}

void PropertiesPanelWidget::showPropertiesFor(const QList<BasePrimitive*>& primitives)
{
    if (primitives.isEmpty()) {
        m_stack->setCurrentWidget(m_emptyWidget);
        return;
    }

    PrimitiveType firstType = primitives.first()->getType();
    bool allSameType = true;
    for(auto* p : primitives) {
        if (p->getType() != firstType) {
            allSameType = false; break;
        }
    }

    if (allSameType) {
        if (firstType == PrimitiveType::Segment) {
            m_segmentProperties->setPrimitives(primitives);
            m_stack->setCurrentWidget(m_segmentProperties);
        }
        else if (firstType == PrimitiveType::Circle) {
            m_circleProperties->setPrimitives(primitives);
            m_stack->setCurrentWidget(m_circleProperties);
        }
        else if (firstType == PrimitiveType::Rectangle) {
            m_rectProperties->setPrimitives(primitives);
            m_stack->setCurrentWidget(m_rectProperties);
        }
        else if (firstType == PrimitiveType::Arc) {
            m_arcProperties->setPrimitives(primitives);
            m_stack->setCurrentWidget(m_arcProperties);
        }
        else {
            m_stack->setCurrentWidget(m_emptyWidget);
        }
    }
    else {
        m_stack->setCurrentWidget(m_emptyWidget);
    }
}

void PropertiesPanelWidget::showPropertiesFor(PrimitiveType type)
{
    if (type == PrimitiveType::Segment) {
        m_segmentProperties->setPrimitives({});
        m_stack->setCurrentWidget(m_segmentProperties);
    }
    else if (type == PrimitiveType::Circle) {
        m_circleProperties->setPrimitives({});
        m_stack->setCurrentWidget(m_circleProperties);
    }
    else if (type == PrimitiveType::Rectangle) {
        m_rectProperties->setPrimitives({});
        m_stack->setCurrentWidget(m_rectProperties);
    }
    else if (type == PrimitiveType::Arc) {
        m_arcProperties->setPrimitives({});
        m_stack->setCurrentWidget(m_arcProperties);
    }
    else {
        m_stack->setCurrentWidget(m_emptyWidget);
    }
}

void PropertiesPanelWidget::setCoordinateSystem(CoordinateSystemType type)
{
    auto* currentWidget = qobject_cast<BasePropertiesWidget*>(m_stack->currentWidget());
    //передача команды о смене системы координат активному виджету свойств
    if (currentWidget) {
        currentWidget->setCoordinateSystem(type);
    }
}

void PropertiesPanelWidget::updateColors()
{
    //передача вызова перекраски всем дочерним виджетам свойств
    m_segmentProperties->updateColors();
    m_circleProperties->updateColors();
    m_rectProperties->updateColors();
    m_arcProperties->updateColors();
}
