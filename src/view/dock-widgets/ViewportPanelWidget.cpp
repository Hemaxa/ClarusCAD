#include "ViewportPanelWidget.h"
#include "Scene.h"
#include "BaseTool.h"
#include "SegmentCreationPrimitive.h"

#include <QPainter>
#include <QMouseEvent>
#include <QEvent>

ViewportPanelWidget::ViewportPanelWidget(const QString& title, QWidget* parent) : BaseDockWidget(title, parent)
{
    //отслеживание мыши для canvas
    canvas()->setMouseTracking(true);

    //отправка информации о взаимодействии с canvas
    canvas()->installEventFilter(this);
}

bool ViewportPanelWidget::eventFilter(QObject* obj, QEvent* event)
{
    //проверка, что событие пришло от холста
    if (obj == canvas()) {
        switch (event->type()) {
        //перерисовка
        case QEvent::Paint:
            //вызов функции перерисовки
            paintCanvas(static_cast<QPaintEvent*>(event));
            return true; //событие обработано

        //нажатие кнопки мыши
        case QEvent::MouseButtonPress: {
            auto* mouseEvent = static_cast<QMouseEvent*>(event);

            //если выбран инструмент, он получает информацию о действие мыши
            if (m_activeTool) m_activeTool->onMousePress(mouseEvent, m_scene, this);

            //вызов перерисовки, т.к. действие инструмента могло изменить сцену
            update();
            return true;
        }

        //перемещение мыши
        case QEvent::MouseMove: {
            auto* mouseEvent = static_cast<QMouseEvent*>(event);

            //если выбран инструмент, он получает информацию о действие мыши
            if (m_activeTool) m_activeTool->onMouseMove(mouseEvent, m_scene, this);

            //вызов перерисовки, т.к. действие инструмента могло изменить сцену
            update();
            return true;
        }

        //отпускание кнопки мыши
        case QEvent::MouseButtonRelease: {
            auto* mouseEvent = static_cast<QMouseEvent*>(event);

            //если выбран инструмент, он получает информацию о действие мыши
            if (m_activeTool) m_activeTool->onMouseRelease(mouseEvent, m_scene, this);

            //вызов перерисовки, т.к. действие инструмента могло изменить сцену
            update();
            return true;
        }

        default:
            break;
        }
    }
    //все остальные события используют стандартный обработчик
    return BaseDockWidget::eventFilter(obj, event);
}

void ViewportPanelWidget::paintCanvas(QPaintEvent* event)
{
    Q_UNUSED(event);

    //создается QPainter для рисования на canvas
    QPainter painter(canvas());

    //включение сглаживания для лучшего качества
    painter.setRenderHint(QPainter::Antialiasing);

    //цвет кисти (В БУДУЩЕМ БУДЕТ ЗАВИСЕТЬ ОТ ТЕМЫ)
    painter.setBrush(QColor(45, 45, 45));
    painter.drawRect(canvas()->rect()); //отрисовка в границах холста

    if (!m_scene) return;

    painter.setPen(Qt::white);
    for (const auto& primitive : m_scene->getPrimitives()) {
        if (primitive->getType() == PrimitiveType::Segment) {
            auto* segment = static_cast<SegmentCreationPrimitive*>(primitive.get());
            painter.drawLine(
                QPointF(segment->getStart().x(), segment->getStart().y()),
                QPointF(segment->getEnd().x(), segment->getEnd().y())
                );
        }
    }

    if (m_activeTool) {
        m_activeTool->onPaint(painter);
    }
}

//обновление содержимого
void ViewportPanelWidget::update() { canvas()->update(); }

//сеттеры
void ViewportPanelWidget::setScene(Scene* scene) { m_scene = scene; }
void ViewportPanelWidget::setActiveTool(BaseTool* tool) { m_activeTool = tool; }
