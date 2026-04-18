#include "PropertiesPanelWidget.h"

#include "SegmentPropertiesWidget.h"
#include "CirclePropertiesWidget.h"
#include "CommonPropertiesWidget.h"
#include "DimensionPropertiesWidget.h"

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
    m_ellipseProperties = new EllipsePropertiesWidget();
    m_polygonProperties = new PolygonPropertiesWidget();
    m_splineProperties = new SplinePropertiesWidget();
    m_dimensionProperties = new DimensionPropertiesWidget();
    m_commonProperties = new CommonPropertiesWidget();

    //добавление содержимого в панель
    m_stack->addWidget(m_emptyWidget);
    m_stack->addWidget(m_segmentProperties);
    m_stack->addWidget(m_circleProperties);
    m_stack->addWidget(m_rectProperties);
    m_stack->addWidget(m_arcProperties);
    m_stack->addWidget(m_ellipseProperties);
    m_stack->addWidget(m_polygonProperties);
    m_stack->addWidget(m_splineProperties);
    m_stack->addWidget(m_dimensionProperties);
    m_stack->addWidget(m_commonProperties);

    auto* layout = new QVBoxLayout(canvas()); //вертикальный шаблон компоновки
    layout->setContentsMargins(0, 0, 0, 0); //убирает отступы, чтобы занять всю допустимую область панели
    layout->addWidget(m_stack); //добавление содержимого панели в canvas

    //сигналы от инструментов пересылаются (пробрасываются) в MainWindow
    //"Отрезок"
    connect(m_segmentProperties, &SegmentPropertiesWidget::propertiesApplied, this, &PropertiesPanelWidget::segmentPropertiesApplied);
    connect(m_segmentProperties, &SegmentPropertiesWidget::colorChanged, this, &PropertiesPanelWidget::colorChanged);
    connect(m_segmentProperties, &SegmentPropertiesWidget::lineTypeChanged, this, &PropertiesPanelWidget::lineTypeChanged);
    connect(m_segmentProperties, &SegmentPropertiesWidget::layerChanged, this, &PropertiesPanelWidget::layerChanged);

    //"Окружность"
    connect(m_circleProperties, &CirclePropertiesWidget::propertiesApplied, this, &PropertiesPanelWidget::circlePropertiesApplied);
    connect(m_circleProperties, &CirclePropertiesWidget::colorChanged, this, &PropertiesPanelWidget::colorChanged);
    connect(m_circleProperties, &CirclePropertiesWidget::lineTypeChanged, this, &PropertiesPanelWidget::lineTypeChanged);
    connect(m_circleProperties, &CirclePropertiesWidget::layerChanged, this, &PropertiesPanelWidget::layerChanged);

    connect(m_rectProperties, &RectanglePropertiesWidget::propertiesApplied, this, &PropertiesPanelWidget::rectanglePropertiesApplied);
    connect(m_rectProperties, &RectanglePropertiesWidget::colorChanged, this, &PropertiesPanelWidget::colorChanged);
    connect(m_rectProperties, &RectanglePropertiesWidget::lineTypeChanged, this, &PropertiesPanelWidget::lineTypeChanged);
    connect(m_rectProperties, &RectanglePropertiesWidget::layerChanged, this, &PropertiesPanelWidget::layerChanged);

    connect(m_arcProperties, &ArcPropertiesWidget::propertiesApplied, this, &PropertiesPanelWidget::arcPropertiesApplied);
    connect(m_arcProperties, &ArcPropertiesWidget::colorChanged, this, &PropertiesPanelWidget::colorChanged);
    connect(m_arcProperties, &ArcPropertiesWidget::lineTypeChanged, this, &PropertiesPanelWidget::lineTypeChanged);
    connect(m_arcProperties, &ArcPropertiesWidget::layerChanged, this, &PropertiesPanelWidget::layerChanged);

    // --- Коннекты ЭЛЛИПСА ---
    connect(m_ellipseProperties, &EllipsePropertiesWidget::propertiesApplied, this, &PropertiesPanelWidget::ellipsePropertiesApplied);
    connect(m_ellipseProperties, &EllipsePropertiesWidget::colorChanged, this, &PropertiesPanelWidget::colorChanged);
    connect(m_ellipseProperties, &EllipsePropertiesWidget::lineTypeChanged, this, &PropertiesPanelWidget::lineTypeChanged);
    connect(m_ellipseProperties, &EllipsePropertiesWidget::layerChanged, this, &PropertiesPanelWidget::layerChanged);

    // --- Коннекты МНОГОУГОЛЬНИКА ---
    connect(m_polygonProperties, &PolygonPropertiesWidget::propertiesApplied, this, &PropertiesPanelWidget::polygonPropertiesApplied);
    connect(m_polygonProperties, &PolygonPropertiesWidget::colorChanged, this, &PropertiesPanelWidget::colorChanged);
    connect(m_polygonProperties, &PolygonPropertiesWidget::lineTypeChanged, this, &PropertiesPanelWidget::lineTypeChanged);
    connect(m_polygonProperties, &PolygonPropertiesWidget::layerChanged, this, &PropertiesPanelWidget::layerChanged);
    connect(m_polygonProperties, &PolygonPropertiesWidget::sidesChanged, this, &PropertiesPanelWidget::polygonSidesChanged);
    connect(m_polygonProperties, &PolygonPropertiesWidget::polygonTypeChanged, this, &PropertiesPanelWidget::polygonTypeChanged);

    // --- Коннекты СПЛАЙНА ---
    connect(m_splineProperties, &SplinePropertiesWidget::propertiesApplied, this, &PropertiesPanelWidget::splinePropertiesApplied);
    connect(m_splineProperties, &SplinePropertiesWidget::colorChanged, this, &PropertiesPanelWidget::colorChanged);
    connect(m_splineProperties, &SplinePropertiesWidget::lineTypeChanged, this, &PropertiesPanelWidget::lineTypeChanged);
    connect(m_splineProperties, &SplinePropertiesWidget::layerChanged, this, &PropertiesPanelWidget::layerChanged);
    connect(m_splineProperties, &SplinePropertiesWidget::closedChanged, this, &PropertiesPanelWidget::splineClosedChanged);

    // --- Коннекты РАЗМЕРОВ ---
    connect(m_dimensionProperties, &DimensionPropertiesWidget::dimensionPropertiesApplied, this, [this](){
        emit this->dimensionPropertiesApplied();
    });

    // --- Коннекты ОБЩИХ СВОЙСТВ ---
    connect(m_commonProperties, &CommonPropertiesWidget::commonPropertiesApplied, this, &PropertiesPanelWidget::commonPropertiesApplied);
    connect(m_commonProperties, &CommonPropertiesWidget::layerChanged, this, &PropertiesPanelWidget::layerChanged);

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
        else if (firstType == PrimitiveType::Ellipse) {
            m_ellipseProperties->setPrimitives(primitives);
            m_stack->setCurrentWidget(m_ellipseProperties);
        }
        else if (firstType == PrimitiveType::Polygon) {
            m_polygonProperties->setPrimitives(primitives);
            m_stack->setCurrentWidget(m_polygonProperties);
        }
        else if (firstType == PrimitiveType::Spline) {
            m_splineProperties->setPrimitives(primitives);
            m_stack->setCurrentWidget(m_splineProperties);
        }
        else if (firstType == PrimitiveType::LinearDimension
                 || firstType == PrimitiveType::RadialDimension
                 || firstType == PrimitiveType::AngularDimension) {
            m_dimensionProperties->setPrimitives(primitives);
            m_stack->setCurrentWidget(m_dimensionProperties);
        }
        else {
            m_stack->setCurrentWidget(m_emptyWidget);
        }
    }
    else {
        // Разные типы - показываем общий виджет
        m_commonProperties->setPrimitives(primitives);
        m_stack->setCurrentWidget(m_commonProperties);
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
    else if (type == PrimitiveType::Ellipse) {
        m_ellipseProperties->setPrimitives({});
        m_stack->setCurrentWidget(m_ellipseProperties);
    }
    else if (type == PrimitiveType::Polygon) {
        m_polygonProperties->setPrimitives({});
        m_stack->setCurrentWidget(m_polygonProperties);
    }
    else if (type == PrimitiveType::Spline) {
        m_splineProperties->setPrimitives({});
        m_stack->setCurrentWidget(m_splineProperties);
    }
    else if (type == PrimitiveType::LinearDimension
             || type == PrimitiveType::RadialDimension
             || type == PrimitiveType::AngularDimension) {
        m_dimensionProperties->setPrimitives({});
        m_stack->setCurrentWidget(m_dimensionProperties);
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
    m_ellipseProperties->updateColors();
    m_polygonProperties->updateColors();
    m_splineProperties->updateColors();
    m_dimensionProperties->updateColors();
    m_commonProperties->updateColors();
}
