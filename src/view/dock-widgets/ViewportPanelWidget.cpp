#include "ViewportPanelWidget.h"
#include "Scene.h"
#include "BaseTool.h"
#include "SegmentCreationPrimitive.h"

#include <QPainter>
#include <QMouseEvent>
#include <QEvent>

ViewportPanelWidget::ViewportPanelWidget(const QString& title, QWidget* parent)
    : BaseDockWidget(title, parent)
{
    // --- НОВАЯ ЛОГИКА ---

    // Включаем отслеживание мыши для холста, а не для всей панели
    canvas()->setMouseTracking(true);

    // Устанавливаем обработчик событий для холста с помощью лямбда-функции.
    // Это позволяет нам "пробросить" события от холста к нашей панели.
    canvas()->installEventFilter(this);
}

// Переопределяем метод фильтра событий
bool ViewportPanelWidget::eventFilter(QObject* obj, QEvent* event)
{
    // Убеждаемся, что событие пришло именно от нашего холста
    if (obj == canvas()) {
        switch (event->type()) {
        case QEvent::Paint:
            // Если холст нужно перерисовать, вызываем нашу функцию отрисовки
            paintCanvas(static_cast<QPaintEvent*>(event));
            return true; // Сообщаем, что мы обработали событие

        case QEvent::MouseButtonPress: {
            auto* mouseEvent = static_cast<QMouseEvent*>(event);
            if (m_activeTool) m_activeTool->onMousePress(mouseEvent, m_scene, this);
            update();
            return true;
        }

        case QEvent::MouseMove: {
            auto* mouseEvent = static_cast<QMouseEvent*>(event);
            if (m_activeTool) m_activeTool->onMouseMove(mouseEvent, m_scene, this);
            update();
            return true;
        }

        case QEvent::MouseButtonRelease: {
            auto* mouseEvent = static_cast<QMouseEvent*>(event);
            if (m_activeTool) m_activeTool->onMouseRelease(mouseEvent, m_scene, this);
            update();
            return true;
        }

        default:
            break;
        }
    }
    // Для всех остальных событий вызываем стандартный обработчик
    return BaseDockWidget::eventFilter(obj, event);
}


// Эта функция теперь рисует НА ХОЛСТЕ
void ViewportPanelWidget::paintCanvas(QPaintEvent* event)
{
    Q_UNUSED(event);

    // Создаем QPainter для холста, а не для this
    QPainter painter(canvas());
    painter.setRenderHint(QPainter::Antialiasing);

    painter.setBrush(QColor(45, 45, 45));
    painter.drawRect(canvas()->rect()); // Рисуем в границах холста

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


// Методы-пробросы остаются простыми
void ViewportPanelWidget::setScene(Scene* scene) { m_scene = scene; }
void ViewportPanelWidget::setActiveTool(BaseTool* tool) { m_activeTool = tool; }
void ViewportPanelWidget::update() { canvas()->update(); }
