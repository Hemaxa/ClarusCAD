#include "PropertiesPanelWidget.h"

// КРИТИЧЕСКИ ВАЖНО: Подключаем "чертежи" для всех классов,
// которые мы здесь создаем и используем.
#include "SegmentPropertiesWidget.h"
#include "BasePrimitive.h"
#include <QStackedWidget>

PropertiesPanelWidget::PropertiesPanelWidget(const QString& title, QWidget* parent)
    : BaseDockWidget(title, parent)
{
    m_stack = new QStackedWidget();

    // Создаем все возможные "начинки"
    m_segmentProperties = new SegmentPropertiesWidget();
    m_emptyWidget = new QWidget();

    m_stack->addWidget(m_emptyWidget);
    m_stack->addWidget(m_segmentProperties);

    // Вставляем "стопку" с начинками в нашу "рамку"
    setWidget(m_stack);

    // Соединяем сигнал от "начинки" с сигналом нашей "рамки",
    // чтобы передать его дальше в MainWindow
    connect(m_segmentProperties, &SegmentPropertiesWidget::createSegmentRequested,
            this, &PropertiesPanelWidget::createSegmentRequested);
}

void PropertiesPanelWidget::showPropertiesFor(BasePrimitive* primitive)
{
    if (!primitive) {
        m_stack->setCurrentWidget(m_emptyWidget);
        return;
    }

    if (primitive->getType() == PrimitiveType::Segment) {
        m_segmentProperties->setPrimitive(primitive);
        m_stack->setCurrentWidget(m_segmentProperties);
    } else {
        m_stack->setCurrentWidget(m_emptyWidget);
    }
}
