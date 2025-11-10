// src/view/ViewportCamera.cpp

#include "ViewportCamera.h"
#include <QtMath>
#include <algorithm> // for std::min, std::max

ViewportCamera::ViewportCamera(QObject* parent) : QObject(parent)
{
    //начальная позиция камеры
    m_panOffset = QPointF(50.0, 50.0);

    //создание и настройка анимации вращения
    m_rotationAnimation = new QPropertyAnimation(this, "rotationAngle", this);
    m_rotationAnimation->setDuration(300); //длительность анимации
    m_rotationAnimation->setEasingCurve(QEasingCurve::OutCubic);

    connect(m_rotationAnimation, &QPropertyAnimation::finished, this, &ViewportCamera::updated);
}

void ViewportCamera::setCanvasSize(const QSize& size)
{
    m_canvasSize = size;
}

QTransform ViewportCamera::getWorldToScreenTransform() const
{
    QTransform t;
    // 6. Переносим в левый нижний угол
    t.translate(0, m_canvasSize.height());
    // 5. Инвертируем Y
    t.scale(1, -1);
    // 4. Масштабируем
    t.scale(m_zoomFactor, m_zoomFactor);
    // 3. Панорамируем (в "повернутом" пространстве)
    t.translate(m_panOffset.x(), m_panOffset.y());
    // 2. Вращаем
    t.rotate(m_rotationAngle);
    // 1. (Входная точка - мировые координаты)
    return t;
}

QTransform ViewportCamera::getScreenToWorldTransform() const
{
    bool invertible;
    return getWorldToScreenTransform().inverted(&invertible);
}

qreal ViewportCamera::getZoomFactor() const
{
    return m_zoomFactor;
}

void ViewportCamera::pan(const QPointF& screenDelta)
{
    QTransform screenToWorldTf = getScreenToWorldTransform();
    if (!screenToWorldTf.isInvertible()) return;

    QPointF worldP0 = screenToWorldTf.map(QPointF(0.0, 0.0));
    QPointF worldP1 = screenToWorldTf.map(screenDelta);
    QPointF worldDelta = worldP1 - worldP0;

    QTransform worldToPanOffsetTransform;
    worldToPanOffsetTransform.rotate(m_rotationAngle);
    QPointF panOffsetDelta = worldToPanOffsetTransform.map(worldDelta);

    m_panOffset += panOffsetDelta;
    emit updated();
}

void ViewportCamera::panWorld(const QPointF& worldDelta)
{
    QTransform worldToPanOffsetTransform;
    worldToPanOffsetTransform.rotate(m_rotationAngle);
    QPointF panOffsetDelta = worldToPanOffsetTransform.map(worldDelta);

    m_panOffset += panOffsetDelta;
    emit updated();
}

void ViewportCamera::applyZoom(double factor, const QPoint& anchorPoint)
{
    QPointF worldPosBeforeZoom = getScreenToWorldTransform().map(anchorPoint);

    m_zoomFactor *= factor;
    m_zoomFactor = std::max(0.05, std::min(m_zoomFactor, 50.0));

    QPointF worldPosAfterZoom = getScreenToWorldTransform().map(anchorPoint);

    // Компенсируем смещение (в мировом пространстве)
    QPointF worldDelta = worldPosBeforeZoom - worldPosAfterZoom;

    // Конвертируем в PanOffset Space и применяем
    QTransform worldToPanOffsetTransform;
    worldToPanOffsetTransform.rotate(m_rotationAngle);
    m_panOffset += worldToPanOffsetTransform.map(worldDelta);

    emit updated();
}

void ViewportCamera::rotate()
{
    m_targetRotationStep = (m_targetRotationStep + 1);
    qreal targetAngle = m_targetRotationStep * 90.0;

    m_rotationAnimation->stop();
    m_rotationAnimation->setEndValue(targetAngle);
    m_rotationAnimation->start();
}

void ViewportCamera::fitBounds(const QRectF& worldBounds)
{
    if (!worldBounds.isValid() || m_canvasSize.isEmpty()) return;

    // 1. Добавляем отступ
    qreal padding = worldBounds.width() * 0.1;
    if (padding < 10) padding = 10; // Минимальный отступ
    QRectF paddedBounds = worldBounds.adjusted(-padding, -padding, padding, padding);

    // 2. Получаем AABB *повернутых* границ
    QTransform rotation;
    rotation.rotate(m_rotationAngle);
    QRectF rotatedBounds = rotation.mapRect(paddedBounds);
    if (rotatedBounds.width() == 0 || rotatedBounds.height() == 0) return;

    // 3. Считаем новый зум
    double xZoom = (double)m_canvasSize.width() / rotatedBounds.width();
    double yZoom = (double)m_canvasSize.height() / rotatedBounds.height();
    m_zoomFactor = std::min(xZoom, yZoom);
    m_zoomFactor = std::max(0.05, std::min(m_zoomFactor, 50.0)); // Ограничиваем

    // 4. Считаем новый panOffset
    // Цель: центр rotatedBounds должен оказаться в центре экрана.

    // Центр экрана в "нашей" системе (Y-up, 0,0 внизу-слева)
    // после инверсии зума:
    QPointF targetPanOffsetSpaceCenter = QPointF(
        (m_canvasSize.width() / 2.0) / m_zoomFactor,
        (m_canvasSize.height() / 2.0) / m_zoomFactor
        );

    // Формула: Target = Current + PanOffset
    // PanOffset = Target - Current
    m_panOffset = targetPanOffsetSpaceCenter - rotatedBounds.center();

    emit updated();
}

// --- Сеттер/Геттер для Q_PROPERTY ---
void ViewportCamera::setRotationAngle(qreal angle)
{
    m_rotationAngle = angle;
    emit updated();
}

qreal ViewportCamera::getRotationAngle() const
{
    return m_rotationAngle;
}
