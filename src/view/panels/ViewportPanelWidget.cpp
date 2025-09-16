#include "ViewportPanelWidget.h"
#include "Scene.h"
#include "BaseCreationTool.h"
#include "BaseDrawingTool.h"

#include <QPainter>
#include <QMouseEvent>
#include <QEvent>

ViewportPanelWidget::ViewportPanelWidget(const QString& title, QWidget* parent) : BasePanelWidget(title, parent)
{
    //включение отслеживания действий мыши
    canvas()->setMouseTracking(true);

    //включение "фильтра событий" на холст, чтобы все события холста проходили через метод eventFilter
    canvas()->installEventFilter(this);

    //позволяет виджету получать фокус (например, по клику), что необходимо для обработки нажатий клавиш
    setFocusPolicy(Qt::StrongFocus);

    //курсор по умолчанию - стрелка
    canvas()->setCursor(Qt::ArrowCursor);

    //начальная позиция камеры
    m_panOffset = QPointF(50.0, 50.0);
}

bool ViewportPanelWidget::eventFilter(QObject* obj, QEvent* event)
{
    //проверка, что событие пришло именно от холста (canvas), а не от другого элемента
    if (obj == canvas()) {
        //используется switch для определения типа события
        switch (event->type()) {
        //событие перерисовки
        case QEvent::Paint:
            paintCanvas(static_cast<QPaintEvent*>(event));
            return true;

        //событие нажатия клавиши мыши
        case QEvent::MouseButtonPress: {
            //преобразование общего события QEvent в более конкретный QMouseEvent
            auto* mouseEvent = static_cast<QMouseEvent*>(event);
            //если нажата ЛКМ и нет активного инструмента (перемещение по холсту)
            if (mouseEvent->button() == Qt::LeftButton && !m_activeTool) {
                m_isPanning = true; //включение режима панорамирования
                m_lastPanPos = mouseEvent->pos(); //сохранение начальной позиции мыши
                canvas()->setCursor(Qt::ClosedHandCursor); //изменение курсора на руку
            }
            //если есть активный инструмент (создание объекта)
            else if (m_activeTool) {
                QPointF worldPos = screenToWorld(mouseEvent->pos());
                QMouseEvent transformedEvent(mouseEvent->type(), worldPos, mouseEvent->globalPosition(), mouseEvent->button(), mouseEvent->buttons(), mouseEvent->modifiers());
                m_activeTool->onMousePress(&transformedEvent, m_scene, this);
            }
            update();
            return true;
        }

        //событие перемещения мыши
        case QEvent::MouseMove: {
            auto* mouseEvent = static_cast<QMouseEvent*>(event);
            //если активен режим панорамирования
            if (m_isPanning) {
                QPoint delta = mouseEvent->pos() - m_lastPanPos; //вычисляется смещение мыши
                m_lastPanPos = mouseEvent->pos(); //обновляется последняя позиция
                m_panOffset += QPointF(delta.x() / m_zoomFactor, -delta.y() / m_zoomFactor); //добавление смещения к общему смещению вида, скорректировав на зум
                update(); //прерисовка
            }
            //если есть активный инструмент
            else if (m_activeTool) {
                QPointF worldPos = screenToWorld(mouseEvent->pos());
                QMouseEvent transformedEvent(mouseEvent->type(), worldPos, mouseEvent->globalPosition(), mouseEvent->button(), mouseEvent->buttons(), mouseEvent->modifiers());
                m_activeTool->onMouseMove(&transformedEvent, m_scene, this);
                update();
            }
            return true;
        }

        //событие отпускания мыши
        case QEvent::MouseButtonRelease: {
            auto* mouseEvent = static_cast<QMouseEvent*>(event);
            if (mouseEvent->button() == Qt::LeftButton && m_isPanning) {
                m_isPanning = false;
                canvas()->setCursor(Qt::ArrowCursor);
            }

            if (m_activeTool) {
                QPointF worldPos = screenToWorld(mouseEvent->pos());
                QMouseEvent transformedEvent(mouseEvent->type(), worldPos, mouseEvent->globalPosition(), mouseEvent->button(), mouseEvent->buttons(), mouseEvent->modifiers());
                m_activeTool->onMouseRelease(&transformedEvent, m_scene, this);
            }
            update();
            return true;
        }

        default:
            break;
        }
    }
    return BasePanelWidget::eventFilter(obj, event);
}

void ViewportPanelWidget::applyZoom(double factor, const QPoint& anchorPoint)
{
    // Вместо центра экрана используем переданную позицию курсора (anchorPoint)
    QPointF worldPosBeforeZoom = screenToWorld(anchorPoint);

    m_zoomFactor *= factor;
    // Более строгие и понятные границы для зума
    m_zoomFactor = std::max(0.05, std::min(m_zoomFactor, 50.0));

    QPointF worldPosAfterZoom = screenToWorld(anchorPoint);

    m_panOffset += worldPosBeforeZoom - worldPosAfterZoom;

    update();
}

void ViewportPanelWidget::paintGrid(QPainter& painter)
{
    QPen gridPen(QColor(60, 60, 60), 1.0); //перо для обычной сетки
    QPen axisXPen(Qt::red, 1.5); //перо для оси X
    QPen axisYPen(Qt::green, 1.5); //перо для оси Y

    painter.setPen(gridPen);

    const int width = canvas()->width();
    const int height = canvas()->height();

    QPointF topLeft = screenToWorld(QPoint(0, 0));
    QPointF bottomRight = screenToWorld(QPoint(width, height));

    //рассчет видимого размера шага сетки в пикселях
    double visualGridStep = m_gridStep * m_zoomFactor;

    //корректировка шага, чтобы он оставался в комфортных пределах
    const double dynamicGridStep = calculateDynamicGridStep();

    double startX = std::floor(topLeft.x() / dynamicGridStep) * dynamicGridStep;
    double startY = std::floor(bottomRight.y() / dynamicGridStep) * dynamicGridStep;

    //отрисовка вертикальных линий
    for (double x = startX; x <= bottomRight.x(); x += dynamicGridStep) {
        if (std::abs(x) < 1e-9) {
            painter.setPen(axisYPen);
        }
        else {
            painter.setPen(gridPen);
        }
        QPointF p1 = worldToScreen(QPointF(x, topLeft.y()));
        QPointF p2 = worldToScreen(QPointF(x, bottomRight.y()));
        painter.drawLine(p1, p2);
    }

    //отрисовка горизонтальных линий
    for (double y = startY; y <= topLeft.y(); y += dynamicGridStep) {
        if (std::abs(y) < 1e-9) {
            painter.setPen(axisXPen);
        } else {
            painter.setPen(gridPen);
        }
        QPointF p1 = worldToScreen(QPointF(topLeft.x(), y));
        QPointF p2 = worldToScreen(QPointF(bottomRight.x(), y));
        painter.drawLine(p1, p2);
    }

    //отрисовка тоцки в центре координат
    QPointF originPointScreen = worldToScreen(QPointF(0.0, 0.0));
    painter.setPen(Qt::white);
    painter.setBrush(Qt::white);
    painter.drawEllipse(originPointScreen, 3, 3);
}

void ViewportPanelWidget::paintGizmo(QPainter& painter)
{
    painter.save(); //сохранение текущих трансформаций
    painter.resetTransform(); //сбрасывание всех трансформаций, чтобы рисовать в экранных координатах
    painter.setRenderHint(QPainter::Antialiasing); //сглаживание для красивых стрелок

    int gizmoSize = 50; //длина оси
    int padding = 15; //отступ от края окна
    QPoint origin(padding, canvas()->height() - padding); //начало координат гизмо

    //ось X
    painter.setPen(QPen(Qt::red, 2.0));
    painter.setBrush(Qt::red);
    painter.drawLine(origin, origin + QPoint(gizmoSize, 0));

    QPolygonF arrowHeadX;
    arrowHeadX << QPointF(origin.x() + gizmoSize, origin.y())
               << QPointF(origin.x() + gizmoSize - 8, origin.y() - 4)
               << QPointF(origin.x() + gizmoSize - 8, origin.y() + 4);
    painter.drawPolygon(arrowHeadX);

    //ось Y
    painter.setPen(QPen(Qt::green, 2.0));
    painter.setBrush(Qt::green);
    painter.drawLine(origin, origin - QPoint(0, gizmoSize));

    QPolygonF arrowHeadY;
    arrowHeadY << QPointF(origin.x(), origin.y() - gizmoSize)
               << QPointF(origin.x() - 4, origin.y() - gizmoSize + 8)
               << QPointF(origin.x() + 4, origin.y() - gizmoSize + 8);
    painter.drawPolygon(arrowHeadY);

    //точка в начале координат
    painter.setPen(Qt::black);
    painter.setBrush(Qt::black);
    painter.drawEllipse(origin, 3, 3);

    //подписи осей
    painter.setPen(Qt::white);
    painter.setBrush(Qt::NoBrush);
    painter.drawText(origin + QPoint(gizmoSize + 8, 4), "X");
    painter.drawText(origin - QPoint(4, gizmoSize + 8), "Y");

    painter.restore(); //восстановление состояния QPainter
}

void ViewportPanelWidget::paintCanvas(QPaintEvent* event)
{
    Q_UNUSED(event);
    QPainter painter(canvas());
    painter.setRenderHint(QPainter::Antialiasing);
    painter.fillRect(canvas()->rect(), QColor(45, 45, 45));
    paintGrid(painter);
    paintGizmo(painter);
    if (!m_scene || !m_drawingTools) return;
    painter.setTransform(QTransform()
                             .translate(0, canvas()->height())
                             .scale(1, -1)
                             .scale(m_zoomFactor, m_zoomFactor)
                             .translate(m_panOffset.x(), m_panOffset.y()));
    for (const auto& primitive : m_scene->getPrimitives()) {
        PrimitiveType type = primitive->getType();
        auto it = m_drawingTools->find(type);
        if (it != m_drawingTools->end()) {
            const auto& drawer = it->second;
            drawer->draw(painter, primitive.get());
        }
    }
    if (m_activeTool) {
        m_activeTool->onPaint(painter);
    }
}

double ViewportPanelWidget::calculateDynamicGridStep() const
{
    double visualGridStep = m_gridStep * m_zoomFactor;
    double dynamicGridStep = m_gridStep;

    // Корректировка шага, чтобы он оставался в комфортных пределах
    while (visualGridStep < 50) {
        dynamicGridStep *= 2;
        visualGridStep *= 2;
    }
    while (visualGridStep > 100) {
        dynamicGridStep /= 2;
        visualGridStep /= 2;
    }
    return dynamicGridStep;
}

QPointF ViewportPanelWidget::snapToGrid(const QPointF& worldPos) const
{
    // Если привязка выключена, просто возвращаем исходную позицию
    if (!m_isGridSnapEnabled) { // <-- Добавьте эту проверку
        return worldPos;
    }
    double dynamicGridStep = getDynamicGridStep();
    if (dynamicGridStep == 0.0) {
        return worldPos;
    }
    double snappedX = std::round(worldPos.x() / dynamicGridStep) * dynamicGridStep;
    double snappedY = std::round(worldPos.y() / dynamicGridStep) * dynamicGridStep;
    return QPointF(snappedX, snappedY);
}

QPointF ViewportPanelWidget::worldToScreen(const QPointF& worldPos) const
{
    double screenX = (worldPos.x() + m_panOffset.x()) * m_zoomFactor;
    double screenY = (worldPos.y() + m_panOffset.y()) * m_zoomFactor;
    return QPointF(screenX, canvas()->height() - screenY);
}

QPointF ViewportPanelWidget::screenToWorld(const QPointF& screenPos) const
{
    double invertedY = canvas()->height() - screenPos.y();
    double worldX = (screenPos.x() / m_zoomFactor) - m_panOffset.x();
    double worldY = (invertedY / m_zoomFactor) - m_panOffset.y();
    return QPointF(worldX, worldY);
}

void ViewportPanelWidget::update() { canvas()->update(); }
void ViewportPanelWidget::setScene(Scene* scene) { m_scene = scene; }
void ViewportPanelWidget::setActiveTool(BaseCreationTool* tool) { m_activeTool = tool; }
void ViewportPanelWidget::setDrawingTools(const std::map<PrimitiveType, std::unique_ptr<BaseDrawingTool>>* tools) { m_drawingTools = tools; }
void ViewportPanelWidget::setGridStep(int step) { if (step > 0) { m_gridStep = step; update(); } }
void ViewportPanelWidget::setGridSnapEnabled(bool enabled) { m_isGridSnapEnabled = enabled; }

double ViewportPanelWidget::getDynamicGridStep() const { return calculateDynamicGridStep(); }
int ViewportPanelWidget::getGridStep() const { return m_gridStep; }
QWidget* ViewportPanelWidget::getCanvas() const { return canvas(); }
double ViewportPanelWidget::getZoomFactor() const { return m_zoomFactor; }
