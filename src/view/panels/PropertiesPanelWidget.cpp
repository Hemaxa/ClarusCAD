#include "PropertiesPanelWidget.h"
#include "SegmentPropertiesWidget.h"
#include "BasePrimitive.h"

#include <QStackedWidget>
#include <QVBoxLayout>

PropertiesPanelWidget::PropertiesPanelWidget(const QString& title, QWidget* parent) : BasePanelWidget(title, parent)
{
    //инициализация самой панели виджетов через QStackedWidget
    m_stack = new QStackedWidget();

    //инициализация содержимого для панели виджетов (переключаемых страниц)
    m_segmentProperties = new SegmentPropertiesWidget();
    m_emptyWidget = new QWidget();

    //добавление содержимого в панель
    m_stack->addWidget(m_segmentProperties);
    m_stack->addWidget(m_emptyWidget);

    auto* layout = new QVBoxLayout(canvas()); //вертикальный шаблон компоновки
    layout->setContentsMargins(0, 0, 0, 0); //убирает отступы, чтобы занять всю допустимую область панели
    layout->addWidget(m_stack); //добавление содержимого панели в canvas

    //сигналы от инструмента "Отрезок" пересылается (пробрасывается) в MainWindow
    connect(m_segmentProperties, &SegmentPropertiesWidget::propertiesApplied, this, &PropertiesPanelWidget::segmentPropertiesApplied);
    connect(m_segmentProperties, &SegmentPropertiesWidget::colorChanged, this, &PropertiesPanelWidget::colorChanged);
    connect(m_segmentProperties, &SegmentPropertiesWidget::lineTypeChanged, this, &PropertiesPanelWidget::lineTypeChanged);

    //минимальная высота окна
    setMinimumHeight(200);
}

void PropertiesPanelWidget::showPropertiesFor(const QList<BasePrimitive*>& primitives)
{
    //если список пуст, показывается пустой виджет
    if (primitives.isEmpty()) {
        m_stack->setCurrentWidget(m_emptyWidget);
        return;
    }

    //Проверка: все ли объекты одного типа?
    //Пока что у нас только "Отрезок", но на будущее
    PrimitiveType firstType = primitives.first()->getType();
    bool allSameType = true;
    for(auto* p : primitives) {
        if (p->getType() != firstType) {
            allSameType = false;
            break;
        }
    }

    if (allSameType && firstType == PrimitiveType::Segment) {
        //Передаем весь список
        m_segmentProperties->setPrimitives(primitives);
        m_stack->setCurrentWidget(m_segmentProperties);
    }
    else {
        //Если типы разные или не поддерживаются - пустой виджет
        m_stack->setCurrentWidget(m_emptyWidget);
    }
}

void PropertiesPanelWidget::showPropertiesFor(PrimitiveType type)
{
    if (type == PrimitiveType::Segment) {
        //Передаем пустой список (вместо nullptr) для режима создания
        m_segmentProperties->setPrimitives({});
        //включается отображение параметров объекта
        m_stack->setCurrentWidget(m_segmentProperties);
    }
    else {
        //для других типов объектов показывается пустрой виджет
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
}
