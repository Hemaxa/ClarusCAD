#include "ViewportPanelWidget.h"
#include "ViewportCamera.h"
#include "Scene.h"
#include "ThemeManager.h"
#include "BaseCreationTool.h"
#include "ObjectBindingManager.h" // [NEW] Подключаем менеджер привязок

#include "BasePrimitive.h"
// Инклуды конкретных примитивов не обязательны для отрисовки, так как используется полиморфизм BasePrimitive->draw()
// Но они могут понадобиться, если где-то нужны cast-ы (в будущем коде стараемся избегать)

#include <QPainter>
#include <QMouseEvent>
#include <QEvent>
#include <QLabel>
#include <QGridLayout>
#include <QtMath>

ViewportPanelWidget::ViewportPanelWidget(const QString& title, QWidget* parent) : BasePanelWidget(title, parent)
{
    canvas()->setMouseTracking(true);
    canvas()->installEventFilter(this);
    canvas()->setCursor(Qt::ArrowCursor);
    setFocusPolicy(Qt::StrongFocus);

    m_camera = new ViewportCamera(this);
    connect(m_camera, &ViewportCamera::updated, this, &ViewportPanelWidget::onCameraUpdated);

    // createDrawingTools(); // УДАЛЕНО

    m_infoLabel = new QLabel(canvas());
    m_infoLabel->setObjectName("InfoLabel");
    m_infoLabel->setFixedSize(100, 60);
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
    updateInfoLabel();
    update();
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
                    double clickThreshold = 10.0 / getZoomFactor();

                    if (m_scene) {
                        for(const auto& prim : m_scene->getPrimitives()) {
                            // ПОЛИМОРФИЗМ: используем универсальный hitTest
                            if (prim->hitTest(worldClick, clickThreshold)) {
                                // Берем последний (верхний), но можно добавить логику Z-order
                                newSelection.clear(); // Точечный клик выделяет только один
                                newSelection.append(prim.get());
                            }
                        }
                    }
                }
                else {
                    // ЛОГИКА РАМКИ
                    if (m_scene) {
                        bool isCrossing = (mouseEvent->pos().x() < m_selectionStartPos.x()); // Crossing (зеленая)

                        // Переводим прямоугольник в мировые координаты для проверки
                        QPointF tl = screenToWorld(selectionRect.topLeft());
                        QPointF br = screenToWorld(selectionRect.bottomRight());
                        QRectF worldRect(tl, br);
                        worldRect = worldRect.normalized();

                        for (const auto& prim : m_scene->getPrimitives()) {
                            // ПОЛИМОРФИЗМ: используем методы примитивов
                            if (isCrossing) {
                                if (prim->intersects(worldRect)) {
                                    newSelection.append(prim.get());
                                }
                            } else {
                                if (prim->inside(worldRect)) {
                                    newSelection.append(prim.get());
                                }
                            }
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

void ViewportPanelWidget::paintCanvas(QPaintEvent* event)
{
    Q_UNUSED(event);
    QPainter painter(canvas());
    painter.setRenderHint(QPainter::Antialiasing);

    // 1. Получаем трансформацию
    QTransform worldToScreen = m_camera->getWorldToScreenTransform();

    // 2. Рисуем сетку
    paintGrid(painter, worldToScreen);
    paintGizmo(painter); // Гизмо рисуется поверх сетки, но до примитивов (или после, если нужно поверх всего)

    if (!m_scene) return;

    // 4. Устанавливаем трансформацию для примитивов
    painter.setTransform(worldToScreen);

    // 5. Отрисовка примитивов (УПРОЩЕНО через Smart Model)
    for (const auto& primitive : m_scene->getPrimitives()) {
        // Проверяем, есть ли примитив в списке выделенных
        bool isSelected = m_selectedPrimitives.contains(primitive.get());

        // Примитив сам знает, как себя рисовать!
        primitive->draw(painter, isSelected);
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

        if (m_currentMousePosScreen.x() < m_selectionStartPos.x()) {
            fillColor = QColor(0, 255, 0, 30); // Зеленая
            borderColor = QColor(0, 255, 0, 150);
            QPen pen(borderColor);
            pen.setStyle(Qt::DashLine);
            painter.setPen(pen);
        }
        else {
            fillColor = QColor(0, 0, 255, 30); // Синяя
            borderColor = QColor(0, 0, 255, 150);
            painter.setPen(borderColor);
        }

        painter.setBrush(fillColor);
        painter.drawRect(selectionRect);
    }
}

// ... paintGrid и paintGizmo остаются без изменений (см. исходный код) ...

// ДЕЛЕГИРОВАНИЕ В ObjectBindingManager
QPointF ViewportPanelWidget::getSnappedPoint(const QPointF& worldPos) const
{
    return ObjectBindingManager::instance().getSnappedPoint(worldPos, m_scene, getZoomFactor());
}

void ViewportPanelWidget::setGridSnapEnabled(bool enabled) { ObjectBindingManager::instance().setGridSnap(enabled); }
void ViewportPanelWidget::setPrimitiveSnapEnabled(bool enabled) { ObjectBindingManager::instance().setPrimitiveSnap(enabled); }
void ViewportPanelWidget::setGridStep(int step) {
    if (step > 0) {
        m_gridStep = step;
        ObjectBindingManager::instance().setGridStep(step); // Синхронизируем менеджер
        updateInfoLabel();
        update();
    }
}

double ViewportPanelWidget::calculateDynamicGridStep() const
{
    return ObjectBindingManager::instance().calculateDynamicGridStep(getZoomFactor());
}

// Остальные геттеры/сеттеры без изменений...
void ViewportPanelWidget::setScene(Scene* scene) { m_scene = scene; }
void ViewportPanelWidget::setActiveTool(BaseCreationTool* tool) { m_activeTool = tool; }
void ViewportPanelWidget::setCoordinateSystem(CoordinateSystemType type) { m_coordSystemType = type; updateInfoLabel(); }
void ViewportPanelWidget::setSelectedPrimitive(BasePrimitive* primitive) { m_selectedPrimitive = primitive; update(); }
void ViewportPanelWidget::setZoomStep(double step) { m_zoomStep = step; }
int ViewportPanelWidget::getGridStep() const { return m_gridStep; }
double ViewportPanelWidget::getZoomFactor() const { return m_camera->getZoomFactor(); }
QWidget* ViewportPanelWidget::getCanvas() const { return canvas(); }
void ViewportPanelWidget::update() { canvas()->update(); }
void ViewportPanelWidget::setSelectedPrimitives(const QList<BasePrimitive*>& primitives) { m_selectedPrimitives = primitives; update(); }
QList<BasePrimitive*> ViewportPanelWidget::getSelectedPrimitives() const { return m_selectedPrimitives; }
