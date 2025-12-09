#include "ViewportPanelWidget.h"
#include "ViewportCamera.h"
#include "Scene.h"
#include "ThemeManager.h"
#include "BaseCreationTool.h"

#include "BaseDrawingTool.h"
#include "SegmentDrawingTool.h"
#include "CircleDrawingTool.h"

#include "PointPrimitive.h"
#include "SegmentPrimitive.h"
#include "CirclePrimitive.h"
#include "RectangleDrawingTool.h"
#include "ArcDrawingTool.h"
#include "EllipseDrawingTool.h"

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

            if (getGizmoRect().contains(mouseEvent->pos())) {
                m_camera->rotateLeft();
                return true;
            }

            if (mouseEvent->button() == Qt::LeftButton && !m_activeTool) {
                m_isSelecting = true;
                m_selectionStartPos = mouseEvent->pos();
                m_currentMousePosScreen = mouseEvent->pos();
                canvas()->setCursor(Qt::CrossCursor);
                return true;
            }

            if (mouseEvent->button() == Qt::MiddleButton && !m_activeTool) {
                m_isPanning = true;
                m_lastPanPos = mouseEvent->pos();
                canvas()->setCursor(Qt::ClosedHandCursor);
                return true;
            }

            if (m_activeTool) {
                QPointF worldPos = screenToWorld(mouseEvent->pos());
                QMouseEvent transformedEvent(mouseEvent->type(), worldPos, mouseEvent->globalPosition(), mouseEvent->button(), mouseEvent->buttons(), mouseEvent->modifiers());
                m_activeTool->onMousePress(&transformedEvent, m_scene, this);
            }
            return true;
        }

        case QEvent::MouseMove: {
            auto* mouseEvent = static_cast<QMouseEvent*>(event);
            m_currentMousePosScreen = mouseEvent->pos();

            if (getGizmoRect().contains(mouseEvent->pos())) {
                canvas()->setCursor(Qt::PointingHandCursor);
            }
            else if (!m_isPanning && !m_isSelecting && !m_activeTool) {
                canvas()->setCursor(Qt::ArrowCursor);
            }

            emit mouseMoved(mouseEvent->pos());
            m_currentMouseWorldPos = screenToWorld(mouseEvent->pos());
            updateInfoLabel();

            if (m_isPanning) {
                QPointF delta = mouseEvent->pos() - m_lastPanPos;
                m_lastPanPos = mouseEvent->pos();
                m_camera->pan(delta);
            }
            else if (m_isSelecting) {
                update();
            }
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

                QRect selectionRect = QRect(m_selectionStartPos, mouseEvent->pos()).normalized();
                QList<BasePrimitive*> newSelection;

                // Если клик был точечным (рамка очень мала)
                if (selectionRect.width() < 3 && selectionRect.height() < 3) {
                    QPointF worldClick = screenToWorld(mouseEvent->pos());

                    // Расстояние для клика (в пикселях, переведенное в мировые единицы)
                    double clickThreshold = 10.0 / getZoomFactor();
                    double minDistance = clickThreshold;
                    BasePrimitive* bestCandidate = nullptr;

                    if (m_scene) {
                        for(const auto& prim : m_scene->getPrimitives()) {
                            double dist = std::numeric_limits<double>::max();

                            // 1. Проверка для Отрезков
                            if (prim->getType() == PrimitiveType::Segment) {
                                auto* seg = static_cast<SegmentPrimitive*>(prim.get());
                                QPointF p1(seg->getStart().getX(), seg->getStart().getY());
                                QPointF p2(seg->getEnd().getX(), seg->getEnd().getY());
                                QLineF line(p1, p2);

                                // Математика расстояния точки до отрезка
                                QPointF vecLine = p2 - p1;
                                QPointF vecPoint = worldClick - p1;
                                double lineLen2 = QPointF::dotProduct(vecLine, vecLine);

                                if (lineLen2 == 0.0) {
                                    dist = QLineF(worldClick, p1).length();
                                } else {
                                    double t = QPointF::dotProduct(vecPoint, vecLine) / lineLen2;
                                    if (t < 0.0) dist = QLineF(worldClick, p1).length();
                                    else if (t > 1.0) dist = QLineF(worldClick, p2).length();
                                    else {
                                        QPointF projection = p1 + t * vecLine;
                                        dist = QLineF(worldClick, projection).length();
                                    }
                                }
                            }
                            // 2. Проверка для Точек
                            else if (prim->getType() == PrimitiveType::Point) {
                                auto* pt = static_cast<PointPrimitive*>(prim.get());
                                dist = QLineF(worldClick, QPointF(pt->getX(), pt->getY())).length();
                            }
                            // 3. НОВОЕ: Проверка для Окружностей
                            else if (prim->getType() == PrimitiveType::Circle) {
                                auto* circle = static_cast<CirclePrimitive*>(prim.get());
                                QPointF center(circle->getCenter().getX(), circle->getCenter().getY());
                                double distToCenter = QLineF(worldClick, center).length();
                                dist = std::abs(distToCenter - circle->getRadius());
                            }

                            // Если попали в допуск и это самый близкий объект
                            if (dist < minDistance) {
                                minDistance = dist;
                                bestCandidate = prim.get();
                            }
                        }
                    }

                    if (bestCandidate) {
                        newSelection.append(bestCandidate);
                    }
                }
                else {
                    // ЛОГИКА РАМКИ
                    if (m_scene) {
                        bool isCrossing = (mouseEvent->pos().x() < m_selectionStartPos.x()); // Справа налево (зеленая)
                        for (const auto& prim : m_scene->getPrimitives()) {

                            // Получаем BoundingBox примитива в мировых координатах
                            QRectF bbWorld = prim->getBoundingBox();

                            // Преобразуем bbox в экранные координаты для проверки (грубая проверка)
                            // Или преобразуем рамку в мировые. Лучше преобразуем точки объекта в экранные.

                            // Для ОТРЕЗКОВ (как было)
                            if (prim->getType() == PrimitiveType::Segment) {
                                auto* seg = static_cast<SegmentPrimitive*>(prim.get());
                                QPoint p1Screen = worldToScreen(QPointF(seg->getStart().getX(), seg->getStart().getY())).toPoint();
                                QPoint p2Screen = worldToScreen(QPointF(seg->getEnd().getX(), seg->getEnd().getY())).toPoint();

                                bool p1In = selectionRect.contains(p1Screen);
                                bool p2In = selectionRect.contains(p2Screen);

                                if (isCrossing) {
                                    // Пересечение (простой вариант: если хотя бы одна точка внутри)
                                    // Плюс проверка на пересечение линии с прямоугольником (здесь упрощено)
                                    if (p1In || p2In) newSelection.append(prim.get());
                                    // TODO: Добавить полноценную проверку пересечения линии и Rect
                                } else {
                                    // Window (обе точки внутри)
                                    if (p1In && p2In) newSelection.append(prim.get());
                                }
                            }
                            // НОВОЕ: Для ОКРУЖНОСТЕЙ
                            else if (prim->getType() == PrimitiveType::Circle) {
                                auto* circle = static_cast<CirclePrimitive*>(prim.get());
                                // Проверка Bounding Box окружности
                                // Преобразуем прямоугольник выделения в мировые координаты для сравнения
                                QPointF topLeftWorld = screenToWorld(selectionRect.topLeft());
                                QPointF bottomRightWorld = screenToWorld(selectionRect.bottomRight());
                                QRectF selectionWorld(topLeftWorld, bottomRightWorld);
                                selectionWorld = selectionWorld.normalized();

                                QRectF circleBB = circle->getBoundingBox();

                                if (isCrossing) {
                                    // Пересечение: если прямоугольники пересекаются
                                    if (selectionWorld.intersects(circleBB)) {
                                        newSelection.append(prim.get());
                                    }
                                } else {
                                    // Window: если bounding box окружности полностью внутри рамки
                                    if (selectionWorld.contains(circleBB)) {
                                        newSelection.append(prim.get());
                                    }
                                }
                            }
                            // Можно добавить поддержку других типов
                        }
                    }
                }

                if (mouseEvent->modifiers() & Qt::ControlModifier) {
                    for(auto* p : newSelection) {
                        if (m_selectedPrimitives.contains(p))
                            m_selectedPrimitives.removeAll(p);
                        else
                            m_selectedPrimitives.append(p);
                    }
                } else {
                    m_selectedPrimitives = newSelection;
                }

                emit selectionChanged(m_selectedPrimitives);
                update();
            }

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
    m_drawingTools[PrimitiveType::Circle] = std::make_unique<CircleDrawingTool>();
    m_drawingTools[PrimitiveType::Rectangle] = std::make_unique<RectangleDrawingTool>();
    m_drawingTools[PrimitiveType::Arc] = std::make_unique<ArcDrawingTool>();
    m_drawingTools[PrimitiveType::Ellipse] = std::make_unique<EllipseDrawingTool>();
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

    // Создаем трансформацию для осей гизмо
    // Вращаем в обратную сторону, т.к. Y на экране "вниз"
    QTransform gizmoTransform;
    gizmoTransform.rotate(-m_camera->getRotationAngle());

    QPointF xAxisEnd = gizmoTransform.map(QPointF(gizmoSize, 0));
    QPointF yAxisEnd = gizmoTransform.map(QPointF(0, -gizmoSize)); // (0, -size) т.к. Y "вверх"

    // Ось X
    painter.setPen(QPen(Qt::red, 2.0));
    painter.setBrush(Qt::red);
    painter.drawLine(origin, origin + xAxisEnd.toPoint());

    QPolygonF arrowHeadX;
    arrowHeadX << origin + xAxisEnd
               << origin + gizmoTransform.map(QPointF(gizmoSize - 8, -4)).toPoint()
               << origin + gizmoTransform.map(QPointF(gizmoSize - 8, 4)).toPoint();
    painter.drawPolygon(arrowHeadX);

    // Ось Y
    painter.setPen(QPen(Qt::green, 2.0));
    painter.setBrush(Qt::green);
    painter.drawLine(origin, origin + yAxisEnd.toPoint());

    QPolygonF arrowHeadY;
    arrowHeadY << origin + yAxisEnd
               << origin + gizmoTransform.map(QPointF(-4, -gizmoSize + 8)).toPoint()
               << origin + gizmoTransform.map(QPointF(4, -gizmoSize + 8)).toPoint();
    painter.drawPolygon(arrowHeadY);

    // Точка в начале
    painter.setPen(Qt::white);
    painter.setBrush(Qt::white);
    painter.drawEllipse(origin, 3, 3);

    // Задаем смещение в 12px "наружу" от конца оси ВНУТРИ системы гизмо
    QPointF xTextPos = gizmoTransform.map(QPointF(gizmoSize + 12, 0));
    QPointF yTextPos = gizmoTransform.map(QPointF(0, -gizmoSize - 12));

    // Рисуем текст
    painter.setPen(Qt::white);
    painter.setBrush(Qt::NoBrush);
    painter.drawText(QRectF(origin.x() + xTextPos.x() - 10, origin.y() + xTextPos.y() - 10, 20, 20), Qt::AlignCenter, "X");
    painter.drawText(QRectF(origin.x() + yTextPos.x() - 10, origin.y() + yTextPos.y() - 10, 20, 20), Qt::AlignCenter, "Y");

    painter.restore();
}

QRect ViewportPanelWidget::getGizmoRect() const
{
    int gizmoSize = 50;
    int padding = 60;
    int clickAreaSize = gizmoSize + padding;
    return QRect(0, canvas()->height() - clickAreaSize, clickAreaSize, clickAreaSize);
}

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
        return;
    }

    QRectF worldBounds;
    bool first = true;
    for (const auto& primitive : m_scene->getPrimitives()) {
        QRectF primBounds = primitive->getBoundingBox();
        if (!primBounds.isValid() && primBounds.size().isEmpty()) {
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

    if (worldBounds.width() == 0 && worldBounds.height() == 0) {
        worldBounds.adjust(-50, -50, 50, 50);
    }

    if (!worldBounds.isValid()) return;

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

    if (m_coordSystemType == CoordinateSystemType::Cartesian) {
        infoText = QString("X: %1\nY: %2\nGrid: x%3")
        .arg(m_currentMouseWorldPos.x(), 0, 'f', 2)
            .arg(m_currentMouseWorldPos.y(), 0, 'f', 2)
            .arg(m_gridMultiplier, 0, 'f', 2);
    }
    else {
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
    applyZoom(m_zoomStep, canvas()->rect().center());
}

void ViewportPanelWidget::zoomIn(const QPoint& anchorPoint)
{
    applyZoom(m_zoomStep, anchorPoint);
}

void ViewportPanelWidget::zoomOut()
{
    applyZoom(1.0 / m_zoomStep, canvas()->rect().center());
}

void ViewportPanelWidget::zoomOut(const QPoint& anchorPoint)
{
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
    const double dynamicGridStep = calculateDynamicGridStep();
    if (dynamicGridStep <= 0) {
        return worldPos;
    }

    double snappedX = std::round(worldPos.x() / dynamicGridStep) * dynamicGridStep;
    double snappedY = std::round(worldPos.y() / dynamicGridStep) * dynamicGridStep;
    return QPointF(snappedX, snappedY);
}

QPointF ViewportPanelWidget::snapToPrimitives(const QPointF& worldPos) const
{
    if (!m_scene) {
        return worldPos;
    }

    double minDistance = 10.0 / getZoomFactor();
    QPointF bestSnapPoint = worldPos;

    for (const auto& primitive : m_scene->getPrimitives()) {
        if (primitive->getType() == PrimitiveType::Segment) {
            auto* segment = static_cast<SegmentPrimitive*>(primitive.get());
            QPointF startPoint(segment->getStart().getX(), segment->getStart().getY());
            QPointF endPoint(segment->getEnd().getX(), segment->getEnd().getY());

            if (QLineF(worldPos, startPoint).length() < minDistance) {
                minDistance = QLineF(worldPos, startPoint).length();
                bestSnapPoint = startPoint;
            }

            if (QLineF(worldPos, endPoint).length() < minDistance) {
                minDistance = QLineF(worldPos, endPoint).length();
                bestSnapPoint = endPoint;
            }
        }
        else if (primitive->getType() == PrimitiveType::Circle) {
            auto* circle = static_cast<CirclePrimitive*>(primitive.get());
            // Привязка к центру
            QPointF center(circle->getCenter().getX(), circle->getCenter().getY());
            if (QLineF(worldPos, center).length() < minDistance) {
                minDistance = QLineF(worldPos, center).length();
                bestSnapPoint = center;
            }
            // Можно добавить привязку к квадрантам окружности
        }
    }
    return bestSnapPoint;
}

QPointF ViewportPanelWidget::getSnappedPoint(const QPointF& worldPos) const
{
    QPointF snappedPos = worldPos;

    if (m_isPrimitiveSnapEnabled) {
        snappedPos = snapToPrimitives(worldPos);
        if (snappedPos == worldPos && m_isGridSnapEnabled) {
            snappedPos = snapToGrid(worldPos);
        }
    }
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
