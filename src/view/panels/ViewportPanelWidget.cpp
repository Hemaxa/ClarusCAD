#include "ViewportPanelWidget.h"
#include "ViewportCamera.h"
#include "Scene.h"
#include "BaseCreationTool.h"
#include "BaseDrawingTool.h"
#include "SegmentDrawingTool.h"
#include "PointPrimitive.h"
#include "SegmentPrimitive.h"
#include "ThemeManager.h"

#include <QPainter>
#include <QMouseEvent>
#include <QEvent>
#include <QLabel>
#include <QGridLayout>
#include <QtMath>

ViewportPanelWidget::ViewportPanelWidget(const QString& title, QWidget* parent) : BasePanelWidget(title, parent)
{
    //включение отслеживания действий мыши
    canvas()->setMouseTracking(true);

    //включение "фильтра событий" на холст, чтобы все события холста проходили через метод eventFilter
    canvas()->installEventFilter(this);

    //курсор по умолчанию - стрелка
    canvas()->setCursor(Qt::ArrowCursor);

    //позволяет виджету получать фокус (например, по клику), что необходимо для обработки нажатий клавиш
    setFocusPolicy(Qt::StrongFocus);

    m_camera = new ViewportCamera(this);

    connect(m_camera, &ViewportCamera::updated, this, &ViewportPanelWidget::onCameraUpdated);

    //создание отрисовщиков
    createDrawingTools();

    //создание и настройка info-панели
    m_infoLabel = new QLabel(canvas());
    m_infoLabel->setObjectName("InfoLabel");
    m_infoLabel->setFixedSize(100, 60); //размеры панели
    m_infoLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    auto* layout = new QGridLayout(canvas());
    layout->setContentsMargins(15, 15, 15, 15);
    layout->addWidget(m_infoLabel, 1, 1, Qt::AlignBottom | Qt::AlignRight);
    layout->setRowStretch(0, 1);
    layout->setColumnStretch(0, 1);

    updateInfoLabel();
}

ViewportPanelWidget::~ViewportPanelWidget() = default;

void ViewportPanelWidget::onCameraUpdated()
{
    // Камера говорит, что ее состояние (зум, поворот, смещение) изменилось
    // Нам нужно обновить инфо-панель и перерисовать холст
    updateInfoLabel();
    update(); // (update() вызывает canvas()->update())
}

bool ViewportPanelWidget::eventFilter(QObject* obj, QEvent* event)
{
    //проверка, что событие пришло именно от холста (canvas)
    if (obj == canvas()) {
        switch (event->type()) {
        case QEvent::Resize:
            m_camera->setCanvasSize(canvas()->size());
            break;

        case QEvent::Paint:
            paintCanvas(static_cast<QPaintEvent*>(event));
            return true;

        case QEvent::MouseButtonPress: {
            auto* mouseEvent = static_cast<QMouseEvent*>(event);

            // Проверка нажатия на гизмо
            if (getGizmoRect().contains(mouseEvent->pos())) {
                m_camera->rotateLeft();
                return true;
            }

            // ЛКМ: Начало выделения рамкой (если нет активного инструмента)
            if (mouseEvent->button() == Qt::LeftButton && !m_activeTool) {
                m_isSelecting = true;
                m_selectionStartPos = mouseEvent->pos();
                m_currentMousePosScreen = mouseEvent->pos();
                canvas()->setCursor(Qt::CrossCursor); // Курсор крестиком для выделения
                return true;
            }

            // СКМ (Колесо мыши): Панорамирование
            if (mouseEvent->button() == Qt::MiddleButton && !m_activeTool) {
                m_isPanning = true;
                m_lastPanPos = mouseEvent->pos();
                canvas()->setCursor(Qt::ClosedHandCursor);
                return true;
            }

            // Активный инструмент (например, создание отрезка)
            if (m_activeTool) {
                QPointF worldPos = screenToWorld(mouseEvent->pos());
                QMouseEvent transformedEvent(mouseEvent->type(), worldPos, mouseEvent->globalPosition(), mouseEvent->button(), mouseEvent->buttons(), mouseEvent->modifiers());
                m_activeTool->onMousePress(&transformedEvent, m_scene, this);
            }
            return true;
        }

        case QEvent::MouseMove: {
            auto* mouseEvent = static_cast<QMouseEvent*>(event);
            m_currentMousePosScreen = mouseEvent->pos(); // Обновляем позицию для рамки

            // Проверка наведения на гизмо для смены курсора
            if (getGizmoRect().contains(mouseEvent->pos())) {
                canvas()->setCursor(Qt::PointingHandCursor);
            }
            else if (!m_isPanning && !m_isSelecting && !m_activeTool) {
                canvas()->setCursor(Qt::ArrowCursor);
            }

            emit mouseMoved(mouseEvent->pos());

            //обновление координат в info-панели
            m_currentMouseWorldPos = screenToWorld(mouseEvent->pos());
            updateInfoLabel();

            // Панорамирование (СКМ)
            if (m_isPanning) {
                QPointF delta = mouseEvent->pos() - m_lastPanPos;
                m_lastPanPos = mouseEvent->pos();
                m_camera->pan(delta);
            }
            // Выделение рамкой (ЛКМ)
            else if (m_isSelecting) {
                update(); // Просто перерисовываем, чтобы отрисовалась рамка
            }
            // Активный инструмент
            else if (m_activeTool) {
                QPointF worldPos = screenToWorld(mouseEvent->pos());
                QMouseEvent transformedEvent(mouseEvent->type(), worldPos, mouseEvent->globalPosition(), mouseEvent->button(), mouseEvent->buttons(), mouseEvent->modifiers());
                m_activeTool->onMouseMove(&transformedEvent, m_scene, this);
                update();
            }
            return true;
        }

        case QEvent::MouseButtonRelease: {
            auto* mouseEvent = static_cast<QMouseEvent*>(event);

            // ЗАВЕРШЕНИЕ ВЫДЕЛЕНИЯ РАМКОЙ (ЛКМ)
            if (mouseEvent->button() == Qt::LeftButton && m_isSelecting) {
                m_isSelecting = false;
                canvas()->setCursor(Qt::ArrowCursor);

                // Формируем прямоугольник выделения
                QRect selectionRect = QRect(m_selectionStartPos, mouseEvent->pos()).normalized();

                QList<BasePrimitive*> newSelection;

                // Если клик был точечным (рамка очень мала)
                if (selectionRect.width() < 3 && selectionRect.height() < 3) {
                    // Простой поиск ближайшего объекта по bounding box (упрощенно)
                    QPointF worldClick = screenToWorld(mouseEvent->pos());
                    double minDistance = 10.0 / getZoomFactor();

                    if (m_scene) {
                        for(const auto& prim : m_scene->getPrimitives()) {
                            // Простая проверка: клик внутри BoundingBox
                            // (Для идеальной точности тут нужен полноценный HitTest, как в DeleteTool)
                            QRectF bb = prim->getBoundingBox();
                            // Расширяем BB для удобства клика
                            if (bb.adjusted(-minDistance, -minDistance, minDistance, minDistance).contains(worldClick)) {
                                newSelection.append(prim.get());
                                break; // Выбираем только один (верхний) при клике
                            }
                        }
                    }
                }
                else {
                    // ЛОГИКА РАМКИ
                    if (m_scene) {
                        // Рамка справа налево (Crossing) или слева направо (Window)?
                        bool isCrossing = (mouseEvent->pos().x() < m_selectionStartPos.x());

                        for (const auto& prim : m_scene->getPrimitives()) {
                            if (prim->getType() == PrimitiveType::Segment) {
                                auto* seg = static_cast<SegmentPrimitive*>(prim.get());
                                // Проецируем точки на экран
                                QPoint p1Screen = worldToScreen(QPointF(seg->getStart().getX(), seg->getStart().getY())).toPoint();
                                QPoint p2Screen = worldToScreen(QPointF(seg->getEnd().getX(), seg->getEnd().getY())).toPoint();

                                bool p1In = selectionRect.contains(p1Screen);
                                bool p2In = selectionRect.contains(p2Screen);

                                if (isCrossing) {
                                    // Пересекающая рамка: хотя бы одна точка внутри
                                    if (p1In || p2In) newSelection.append(prim.get());
                                } else {
                                    // Обычная рамка: обе точки строго внутри
                                    if (p1In && p2In) newSelection.append(prim.get());
                                }
                            }
                            // Добавьте else if для других типов (Точки и т.д.)
                        }
                    }
                }

                // Обработка Ctrl (добавление к выделению)
                if (mouseEvent->modifiers() & Qt::ControlModifier) {
                    for(auto* p : newSelection) {
                        if (m_selectedPrimitives.contains(p))
                            m_selectedPrimitives.removeAll(p); // Если был - убираем
                        else
                            m_selectedPrimitives.append(p);    // Если не было - добавляем
                    }
                } else {
                    m_selectedPrimitives = newSelection;
                }

                emit selectionChanged(m_selectedPrimitives); // Сигнал в MainWindow
                update();
            }

            // Завершение панорамирования (СКМ)
            if (mouseEvent->button() == Qt::MiddleButton && m_isPanning) {
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

double ViewportPanelWidget::calculateDynamicGridStep() const
{
    //вычисление шага
    double visualGridStep = m_gridStep * getZoomFactor();
    double dynamicGridStep = m_gridStep;

    //корректировка шага
    while (visualGridStep < 15) {
        dynamicGridStep *= 2;
        visualGridStep *= 2;
    }
    while (visualGridStep > 150) {
        dynamicGridStep /= 2;
        visualGridStep /= 2;
    }
    return dynamicGridStep;
}

void ViewportPanelWidget::createDrawingTools()
{
    m_drawingTools[PrimitiveType::Segment] = std::make_unique<SegmentDrawingTool>();
}

void ViewportPanelWidget::paintCanvas(QPaintEvent* event)
{
    Q_UNUSED(event);
    QPainter painter(canvas());
    painter.setRenderHint(QPainter::Antialiasing);

    // 1. Получаем трансформацию
    QTransform worldToScreen = m_camera->getWorldToScreenTransform();

    // 2. Рисуем сетку
    paintGrid(painter, worldToScreen);

    // 3. Рисуем гизмо
    paintGizmo(painter);

    if (!m_scene) return;

    // 4. Устанавливаем трансформацию для примитивов
    painter.setTransform(worldToScreen);

    // 5. Отрисовка примитивов
    for (const auto& primitive : m_scene->getPrimitives()) {
        PrimitiveType type = primitive->getType();
        auto it = m_drawingTools.find(type);
        if (it != m_drawingTools.end()) {
            const auto& drawer = it->second;
            // Проверяем, есть ли примитив в списке выделенных
            bool isSelected = m_selectedPrimitives.contains(primitive.get());
            drawer->draw(painter, primitive.get(), isSelected);
        }
    }

    // 6. Отрисовка инструмента
    if (m_activeTool) {
        m_activeTool->onPaint(painter);
    }

    // 7. ОТРИСОВКА РАМКИ ВЫДЕЛЕНИЯ
    if (m_isSelecting) {
        painter.resetTransform(); // Рисуем в экранных координатах
        QRect selectionRect = QRect(m_selectionStartPos, m_currentMousePosScreen).normalized();

        QColor fillColor;
        QColor borderColor;

        // Зеленая рамка (Crossing / Справа-налево)
        if (m_currentMousePosScreen.x() < m_selectionStartPos.x()) {
            fillColor = QColor(0, 255, 0, 30);
            borderColor = QColor(0, 255, 0, 150);
            QPen pen(borderColor);
            pen.setStyle(Qt::DashLine);
            painter.setPen(pen);
        }
        // Синяя рамка (Window / Слева-направо)
        else {
            fillColor = QColor(0, 0, 255, 30);
            borderColor = QColor(0, 0, 255, 150);
            painter.setPen(borderColor);
        }

        painter.setBrush(fillColor);
        painter.drawRect(selectionRect);
    }
}

void ViewportPanelWidget::paintGrid(QPainter& painter, const QTransform& worldTransform)
{
    QPen gridPen(QColor(60, 60, 60), 1.0);
    QPen axisXPen(Qt::red, 1.5);
    QPen axisYPen(Qt::green, 1.5);

    painter.save(); // Сохраняем состояние (экранные координаты)
    painter.setTransform(worldTransform); // Применяем мировую трансформацию

    // Определяем видимые границы в МИРОВЫХ координатах
    QTransform screenToWorldTf = worldTransform.inverted();
    QRectF visibleWorldRect = screenToWorldTf.mapRect(canvas()->rect());
    QRectF worldBounds = visibleWorldRect; // AABB

    const double dynamicGridStep = calculateDynamicGridStep();

    double startX = std::floor(worldBounds.left() / dynamicGridStep) * dynamicGridStep;
    double endX = std::ceil(worldBounds.right() / dynamicGridStep) * dynamicGridStep;

    // --- Твой фикс для горизонтальных линий (он правильный) ---
    double startY = std::floor(worldBounds.top() / dynamicGridStep) * dynamicGridStep;
    double endY = std::ceil(worldBounds.bottom() / dynamicGridStep) * dynamicGridStep;

    // --- Отрисовка линий (в мировых координатах) ---
    // Вертикальные линии
    for (double x = startX; x <= endX; x += dynamicGridStep) {
        if (std::abs(x) < 1e-9) painter.setPen(axisYPen);
        else painter.setPen(gridPen);
        painter.drawLine(QPointF(x, startY), QPointF(x, endY));
    }

    // Горизонтальные линии
    for (double y = startY; y <= endY; y += dynamicGridStep) {
        if (std::abs(y) < 1e-9) painter.setPen(axisXPen);
        else painter.setPen(gridPen);
        painter.drawLine(QPointF(startX, y), QPointF(endX, y));
    }

    painter.restore(); // Возвращаемся в экранные координаты

    // --- ИСПРАВЛЕНИЕ 2 (ШКАЛЫ) ---
    // Получаем экранные векторы, указывающие направление осей
    QPointF originScreen = worldTransform.map(QPointF(0.0, 0.0));
    QPointF xAxisScreen = worldTransform.map(QPointF(1.0, 0.0));
    QPointF yAxisScreen = worldTransform.map(QPointF(0.0, 1.0));

    // Получаем векторы
    QPointF xVec = xAxisScreen - originScreen;
    QPointF yVec = yAxisScreen - originScreen;

    // Нормализуем их вручную (нужен #include <QtMath>)
    qreal xLen = qSqrt(xVec.x() * xVec.x() + xVec.y() * xVec.y());
    qreal yLen = qSqrt(yVec.x() * yVec.x() + yVec.y() * yVec.y());

    QPointF xDir(0, 0);
    QPointF yDir(0, 0);

    if (xLen > 1e-6) xDir = xVec / xLen;
    if (yLen > 1e-6) yDir = yVec / yLen;

    // Из-за Y-инверсии нашей матрицы, yDir указывает "вверх" по экрану.
    // Нам нужен вектор "вниз" (перпендикулярно оси X).
    QPointF downDir = -yDir;
    // xDir указывает "вправо" по экрану. Нам нужен "влево" (перпендикулярно оси Y).
    QPointF leftDir = -xDir;
    // --- КОНЕЦ ИСПРАВЛЕНИЯ 2 ---


    // --- Отрисовка текста и точки (0,0) (в экранных координатах) ---
    QColor scaleColor = ThemeManager::instance().getColor("axisScaleColor");
    painter.setFont(QFont("Monaco", 9));

    // Шкала X
    for (double x = startX; x <= endX; x += dynamicGridStep) {
        if (std::abs(x) < 1e-9) continue;

        // 1. Получаем позицию риски на экране
        QPointF tickPosScreen = worldTransform.map(QPointF(x, 0.0));

        // 2. Смещаем ее на 15 пикселей "вниз" (вдоль вектора downDir)
        QPointF textScreenPos = tickPosScreen + (downDir * 15.0);

        painter.save();
        painter.setPen(scaleColor);
        // 3. Рисуем текст в этом месте
        painter.drawText(QRectF(textScreenPos.x() - 25, textScreenPos.y() - 10, 50, 20), Qt::AlignCenter, QString::number(x));
        painter.restore();
    }

    // Шкала Y
    for (double y = startY; y <= endY; y += dynamicGridStep) {
        if (std::abs(y) < 1e-9) continue;

        // 1. Получаем позицию риски на экране
        QPointF tickPosScreen = worldTransform.map(QPointF(0.0, y));

        // 2. Смещаем ее на 15 пикселей "влево" (вдоль вектора leftDir)
        QPointF textScreenPos = tickPosScreen + (leftDir * 15.0);

        painter.save();
        painter.setPen(scaleColor);
        // 3. Рисуем текст в этом месте
        painter.drawText(QRectF(textScreenPos.x() - 55, textScreenPos.y() - 10, 50, 20), Qt::AlignRight | Qt::AlignVCenter, QString::number(y));
        painter.restore();
    }

    // Точка в центре координат
    QPointF originPointScreen = worldTransform.map(QPointF(0.0, 0.0));
    painter.save();
    painter.setPen(Qt::white);
    painter.setBrush(Qt::white);
    painter.drawEllipse(originPointScreen, 3, 3);
    painter.restore();
}

void ViewportPanelWidget::paintGizmo(QPainter& painter)
{
    painter.save();
    painter.resetTransform();
    painter.setRenderHint(QPainter::Antialiasing);

    int gizmoSize = 50;
    int padding = 60;
    QPoint origin(padding, canvas()->height() - padding);

    // --- НОВЫЙ КОД ---
    // Создаем трансформацию для осей гизмо
    // Вращаем в обратную сторону, т.к. Y на экране "вниз"
    QTransform gizmoTransform;
    gizmoTransform.rotate(-m_camera->getRotationAngle()); // <-- Применяем вращение

    QPointF xAxisEnd = gizmoTransform.map(QPointF(gizmoSize, 0));
    QPointF yAxisEnd = gizmoTransform.map(QPointF(0, -gizmoSize)); // (0, -size) т.к. Y "вверх"
    // --- КОНЕЦ НОВОГО КОДА ---

    // Ось X (рисуем до 'rotated' точки)
    painter.setPen(QPen(Qt::red, 2.0));
    painter.setBrush(Qt::red);
    painter.drawLine(origin, origin + xAxisEnd.toPoint());

    QPolygonF arrowHeadX;
    arrowHeadX << origin + xAxisEnd
               << origin + gizmoTransform.map(QPointF(gizmoSize - 8, -4)).toPoint()
               << origin + gizmoTransform.map(QPointF(gizmoSize - 8, 4)).toPoint();
    painter.drawPolygon(arrowHeadX);

    // Ось Y (рисуем до 'rotated' точки)
    painter.setPen(QPen(Qt::green, 2.0));
    painter.setBrush(Qt::green);
    painter.drawLine(origin, origin + yAxisEnd.toPoint());

    QPolygonF arrowHeadY;
    arrowHeadY << origin + yAxisEnd
               << origin + gizmoTransform.map(QPointF(-4, -gizmoSize + 8)).toPoint()
               << origin + gizmoTransform.map(QPointF(4, -gizmoSize + 8)).toPoint();
    painter.drawPolygon(arrowHeadY);

    // ... (точка в начале координат - не меняется) ...
    painter.setPen(Qt::white);
    painter.setBrush(Qt::white);
    painter.drawEllipse(origin, 3, 3);

    // --- ИСПРАВЛЕНИЕ ЗДЕСЬ ---
    // painter.drawText(origin + xAxisEnd.toPoint() + QPoint(8, 4), "X"); // <-- БЫЛО
    // painter.drawText(origin + yAxisEnd.toPoint() + QPoint(-4, -8), "Y"); // <-- БЫЛО

    // Задаем смещение в 12px "наружу" от конца оси ВНУТРИ системы гизмо
    QPointF xTextPos = gizmoTransform.map(QPointF(gizmoSize + 12, 0));
    QPointF yTextPos = gizmoTransform.map(QPointF(0, -gizmoSize - 12));

    // Рисуем текст в прямоугольнике 20x20, центрируя его
    painter.setPen(Qt::white);
    painter.setBrush(Qt::NoBrush);
    painter.drawText(QRectF(origin.x() + xTextPos.x() - 10, origin.y() + xTextPos.y() - 10, 20, 20), Qt::AlignCenter, "X");
    painter.drawText(QRectF(origin.x() + yTextPos.x() - 10, origin.y() + yTextPos.y() - 10, 20, 20), Qt::AlignCenter, "Y");
    // --- КОНЕЦ ИСПРАВЛЕНИЯ ---

    painter.restore();
}

// НОВЫЙ МЕТОД: ОБЛАСТЬ НАЖАТИЯ ГИЗМО
QRect ViewportPanelWidget::getGizmoRect() const
{
    int gizmoSize = 50;
    int padding = 60;
    int clickAreaSize = gizmoSize + padding; // 65
    // Прямоугольник 65x65 в левом нижнем углу
    return QRect(0, canvas()->height() - clickAreaSize, clickAreaSize, clickAreaSize);
}

// НОВЫЙ МЕТОД: СЛОТ ДЛЯ ЗАПУСКА АНИМАЦИИ
void ViewportPanelWidget::rotateSceneLeft()
{
    m_camera->rotateLeft();
}

void ViewportPanelWidget::rotateSceneRight()
{
    m_camera->rotateRight();
}

void ViewportPanelWidget::zoomToExtents()
{
    if (!m_scene || m_scene->getPrimitives().empty()) {
        // Если сцена пуста, можно просто сбросить вид (по желанию)
        // m_camera->resetView(); // (потребовалось бы добавить в камеру)
        return;
    }

    // 1. Считаем общий AABB в Мировом пространстве
    QRectF worldBounds;
    bool first = true;
    for (const auto& primitive : m_scene->getPrimitives()) {
        QRectF primBounds = primitive->getBoundingBox();
        if (!primBounds.isValid() && primBounds.size().isEmpty()) {
            // Для точек (0x0 rect) мы используем united,
            // который правильно обработает их как координаты
            if (first) {
                worldBounds = primBounds;
                first = false;
            } else {
                worldBounds = worldBounds.united(primBounds);
            }
        } else if (primBounds.isValid()) {
            if (first) {
                worldBounds = primBounds;
                first = false;
            } else {
                worldBounds = worldBounds.united(primBounds);
            }
        }
    }

    // Если у нас только одна точка (или несколько в одном месте),
    // worldBounds будет 0x0. Дадим ему мин. размер.
    if (worldBounds.width() == 0 && worldBounds.height() == 0) {
        worldBounds.adjust(-50, -50, 50, 50); // Даем +/- 50
    }

    if (!worldBounds.isValid()) return;

    // 2. Делегируем камере
    m_camera->fitBounds(worldBounds);
}

void ViewportPanelWidget::updateInfoLabel()
{
    QString infoText;

    if (m_gridStep > 0) {
        m_gridMultiplier = calculateDynamicGridStep() / m_gridStep;
    }
    else {
        m_gridMultiplier = 1.0;
    }

    //проверка текущей системы координат
    //декартова система координат
    if (m_coordSystemType == CoordinateSystemType::Cartesian) {
        infoText = QString("X: %1\nY: %2\nGrid: x%3")
        .arg(m_currentMouseWorldPos.x(), 0, 'f', 2)
            .arg(m_currentMouseWorldPos.y(), 0, 'f', 2)
            .arg(m_gridMultiplier, 0, 'f', 2);
    }
    //полярная система координат
    else {
        //перевод координат
        PointPrimitive p(m_currentMouseWorldPos.x(), m_currentMouseWorldPos.y());
            infoText = QString("R: %1\nA: %2°\nGrid: x%3")
            .arg(p.getRadius(), 0, 'f', 2)
            .arg(p.getAngle(), 0, 'f', 2)
            .arg(m_gridMultiplier, 0, 'f', 2);
    }
    m_infoLabel->setText(infoText);
}

void ViewportPanelWidget::applyZoom(double factor, const QPoint& anchorPoint)
{
    m_camera->applyZoom(factor, anchorPoint);
}

void ViewportPanelWidget::zoomIn()
{
    //зум к центру холста
    applyZoom(m_zoomStep, canvas()->rect().center());
}

void ViewportPanelWidget::zoomIn(const QPoint& anchorPoint)
{
    //зум к указанной точке
    applyZoom(m_zoomStep, anchorPoint);
}

void ViewportPanelWidget::zoomOut()
{
    //отдаление от центра холста
    applyZoom(1.0 / m_zoomStep, canvas()->rect().center());
}

void ViewportPanelWidget::zoomOut(const QPoint& anchorPoint)
{
    //отдаление от указанной точки
    applyZoom(1.0 / m_zoomStep, anchorPoint);
}

QPointF ViewportPanelWidget::worldToScreen(const QPointF& worldPos) const
{
    return m_camera->getWorldToScreenTransform().map(worldPos);
}

QPointF ViewportPanelWidget::screenToWorld(const QPointF& screenPos) const
{
    return m_camera->getScreenToWorldTransform().map(screenPos);
}

QPointF ViewportPanelWidget::snapToGrid(const QPointF& worldPos) const
{
    const double dynamicGridStep = calculateDynamicGridStep(); // <-- СТАЛО
    if (dynamicGridStep <= 0) { // <-- СТАЛО
        return worldPos;
    }

    //округление координат
    // double snappedX = std::round(worldPos.x() / m_gridStep) * m_gridStep; // <-- БЫЛО
    // double snappedY = std::round(worldPos.y() / m_gridStep) * m_gridStep; // <-- БЫЛО
    double snappedX = std::round(worldPos.x() / dynamicGridStep) * dynamicGridStep; // <-- СТАЛО
    double snappedY = std::round(worldPos.y() / dynamicGridStep) * dynamicGridStep; // <-- СТАЛО
    // --- КОНЕЦ ИСПРАВЛЕНИЯ ---
    return QPointF(snappedX, snappedY);
}

QPointF ViewportPanelWidget::snapToPrimitives(const QPointF& worldPos) const
{
    if (!m_scene) {
        return worldPos;
    }

    double minDistance = 10.0 / getZoomFactor(); //порог привязки (10 пикселей)
    QPointF bestSnapPoint = worldPos;

    for (const auto& primitive : m_scene->getPrimitives()) {
        if (primitive->getType() == PrimitiveType::Segment) {
            auto* segment = static_cast<SegmentPrimitive*>(primitive.get());
            QPointF startPoint(segment->getStart().getX(), segment->getStart().getY());
            QPointF endPoint(segment->getEnd().getX(), segment->getEnd().getY());

            //проверка расстояния до начальной точки
            if (QLineF(worldPos, startPoint).length() < minDistance) {
                minDistance = QLineF(worldPos, startPoint).length();
                bestSnapPoint = startPoint;
            }

            //проверка расстояния до конечной точки
            if (QLineF(worldPos, endPoint).length() < minDistance) {
                minDistance = QLineF(worldPos, endPoint).length();
                bestSnapPoint = endPoint;
            }
        }
    }
    return bestSnapPoint;
}

QPointF ViewportPanelWidget::getSnappedPoint(const QPointF& worldPos) const
{
    QPointF snappedPos = worldPos;

    //сначала идет привязка к примитивам, если включено
    if (m_isPrimitiveSnapEnabled) {
        snappedPos = snapToPrimitives(worldPos);
        //если не получилась привязка к примитиву и включена привязка к сетке, то пробуется она
        if (snappedPos == worldPos && m_isGridSnapEnabled) {
            snappedPos = snapToGrid(worldPos);
        }
    }
    //елси привязка к примитивам выключена, но включена к сетке
    else if (m_isGridSnapEnabled) {
        snappedPos = snapToGrid(worldPos);
    }

    return snappedPos;
}

void ViewportPanelWidget::panWorld(const QPointF& worldDelta)
{
    m_camera->panWorld(worldDelta);
}

void ViewportPanelWidget::setScene(Scene* scene) { m_scene = scene; }
void ViewportPanelWidget::setActiveTool(BaseCreationTool* tool) { m_activeTool = tool; }
void ViewportPanelWidget::setGridStep(int step) { if (step > 0) { m_gridStep = step; if (m_gridStep > 0) { m_gridMultiplier = calculateDynamicGridStep() / m_gridStep; } updateInfoLabel(); update(); } }
void ViewportPanelWidget::setGridSnapEnabled(bool enabled) { m_isGridSnapEnabled = enabled; }
void ViewportPanelWidget::setPrimitiveSnapEnabled(bool enabled) { m_isPrimitiveSnapEnabled = enabled; }
void ViewportPanelWidget::setCoordinateSystem(CoordinateSystemType type) { m_coordSystemType = type; updateInfoLabel(); }
void ViewportPanelWidget::setSelectedPrimitive(BasePrimitive* primitive) { m_selectedPrimitive = primitive; update(); }
void ViewportPanelWidget::setZoomStep(double step) { m_zoomStep = step; }

int ViewportPanelWidget::getGridStep() const { return m_gridStep; }
double ViewportPanelWidget::getDynamicGridStep() const { return calculateDynamicGridStep(); }
double ViewportPanelWidget::getZoomFactor() const { return m_camera->getZoomFactor(); }
QWidget* ViewportPanelWidget::getCanvas() const { return canvas(); }

void ViewportPanelWidget::update() { canvas()->update(); }

void ViewportPanelWidget::setSelectedPrimitives(const QList<BasePrimitive*>& primitives)
{
    m_selectedPrimitives = primitives;
    update();
}

QList<BasePrimitive*> ViewportPanelWidget::getSelectedPrimitives() const
{
    return m_selectedPrimitives;
}
