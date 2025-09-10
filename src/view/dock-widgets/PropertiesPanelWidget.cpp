#include "PropertiesPanelWidget.h"
#include "SegmentPropertiesWidget.h"
#include "BasePrimitive.h"

#include <QStackedWidget>
#include <QVBoxLayout>

PropertiesPanelWidget::PropertiesPanelWidget(const QString& title, QWidget* parent) : BaseDockWidget(title, parent)
{
    //инициализация самой панели виджетов
    m_stack = new QStackedWidget();

    //инициализация содержимого для панели виджетов
    m_segmentProperties = new SegmentPropertiesWidget();
    m_emptyWidget = new QWidget();

    //добавление содержимого в панель
    m_stack->addWidget(m_emptyWidget);
    m_stack->addWidget(m_segmentProperties);

    auto* layout = new QVBoxLayout(canvas()); //вертикальное расположение параметров
    layout->setContentsMargins(0, 0, 0, 0); //убирает отступы, чтобы занять всю допустимую область панели
    layout->addWidget(m_stack); //добавление содержимого панели на холст

    //сигнал от инструмента "Отрезок" пересылается в MainWindow
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
        m_segmentProperties->setPrimitive(primitive);
        m_stack->setCurrentWidget(m_segmentProperties);
    } else {
        m_stack->setCurrentWidget(m_emptyWidget);
    }
}

void PropertiesPanelWidget::showPropertiesFor(PrimitiveType type)
{
    if (type == PrimitiveType::Segment) {
        m_segmentProperties->setPrimitive(nullptr);
        m_stack->setCurrentWidget(m_segmentProperties);
    } else {
        m_stack->setCurrentWidget(m_emptyWidget);
    }
}
