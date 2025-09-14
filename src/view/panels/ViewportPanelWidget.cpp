#include "ViewportPanelWidget.h"
#include "Scene.h"
#include "BaseCreationTool.h"
#include "BaseDrawingTool.h"

#include <QPainter> //класс Qt для рисования
#include <QMouseEvent> //класс Qt, содержащий информацию о событиях мыши
#include <QEvent> //базовый класс Qt для всех событий

ViewportPanelWidget::ViewportPanelWidget(const QString& title, QWidget* parent) : BasePanelWidget(title, parent)
{
    //отслеживание мыши для canvas, даже если клавиши не нажаты
    canvas()->setMouseTracking(true);

    //отправка информации о взаимодействии с canvas
    canvas()->installEventFilter(this);
}

bool ViewportPanelWidget::eventFilter(QObject* obj, QEvent* event)
{
    //проверка, что событие пришло от холста
    if (obj == canvas()) {
        //switch используется для определения типа события
        switch (event->type()) {

        //базовое событие QEvent* вызывает собственную обработку (QPaintEvent* или QMouseEvent*)
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
    //все остальные события используют стандартный обработчик QEvent*
    return BasePanelWidget::eventFilter(obj, event);
}

void ViewportPanelWidget::paintCanvas(QPaintEvent* event)
{
    Q_UNUSED(event);

    //создается QPainter для рисования на canvas
    QPainter painter(canvas());

    //включение сглаживания для лучшего качества
    painter.setRenderHint(QPainter::Antialiasing);

    //настройка кисти
    painter.setBrush(QColor(45, 45, 45));
    painter.drawRect(canvas()->rect()); //отрисовка в границах холста

    //если нет сцены или инструментов для рисования, отрисовка прекращается
    if (!m_scene || !m_drawingTools) return;

    //проход по каждому объекту в сцене
    for (const auto& primitive : m_scene->getPrimitives()) {
        PrimitiveType type = primitive->getType();

        //ищется нужный отрисовщик в реестре, соответствующий типу примитива
        auto it = m_drawingTools->find(type);

        //если отрисовщик найден, то происходит обращение по указателю для отрисовки
        if (it != m_drawingTools->end()) {
            const auto& drawer = it->second;
            drawer->draw(painter, primitive.get());
        }
    }

    //отрисовка вспомогательной геометрии
    if (m_activeTool) {
        m_activeTool->onPaint(painter);
    }
}

void ViewportPanelWidget::update() { canvas()->update(); }

void ViewportPanelWidget::setScene(Scene* scene) { m_scene = scene; }
void ViewportPanelWidget::setActiveTool(BaseCreationTool* tool) { m_activeTool = tool; }
void ViewportPanelWidget::setDrawingTools(const std::map<PrimitiveType, std::unique_ptr<BaseDrawingTool>>* tools) { m_drawingTools = tools; }
