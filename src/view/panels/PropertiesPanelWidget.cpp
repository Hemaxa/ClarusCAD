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

    //сигнал от инструмента "Отрезок" пересылается (пробрасывается) в MainWindow
    connect(m_segmentProperties, &SegmentPropertiesWidget::createSegmentRequested, this, &PropertiesPanelWidget::createSegmentRequested);

    //минимальная высота окна
    setMinimumHeight(200);
}

void PropertiesPanelWidget::showPropertiesFor(BasePrimitive* primitive)
{
    //если указатель nullptr, показывается пустой виджет
    if (!primitive) {
        m_stack->setCurrentWidget(m_emptyWidget);
        return;
    }

    //иначе параметры объекта, на который ссылается указатель
    if (primitive->getType() == PrimitiveType::Segment) {
        //указатель на объект передается в setPrimitive в SegmentPropertiesWidget
        m_segmentProperties->setPrimitive(primitive);
        //включается отображение параметров объекта
        m_stack->setCurrentWidget(m_segmentProperties);
    }
    else {
        //для других типов объектов показывается пустрой виджет
        m_stack->setCurrentWidget(m_emptyWidget);
    }
}

void PropertiesPanelWidget::showPropertiesFor(PrimitiveType type)
{
    if (type == PrimitiveType::Segment) {
        //пустой указатель на объект передается в setPrimitive в SegmentPropertiesWidget (для создания нового объекта)
        m_segmentProperties->setPrimitive(nullptr);
        //включается отображение параметров объекта
        m_stack->setCurrentWidget(m_segmentProperties);
    }
    else {
        //для других типов объектов показывается пустрой виджет
        m_stack->setCurrentWidget(m_emptyWidget);
    }
}
