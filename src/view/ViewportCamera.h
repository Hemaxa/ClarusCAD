//ViewportCamera - класс камеры для ViewportPanelWidget

#pragma once

#include <QObject>
#include <QPointF>
#include <QSize>
#include <QTransform>
#include <QPropertyAnimation>

class ViewportCamera : public QObject
{
    Q_OBJECT
    Q_PROPERTY(qreal rotationAngle READ getRotationAngle WRITE setRotationAngle)

public:
    explicit ViewportCamera(QObject* parent = nullptr);

    // --- Управление состоянием ---
    void setCanvasSize(const QSize& size);
    void pan(const QPointF& screenDelta);
    void panWorld(const QPointF& worldDelta);
    void applyZoom(double factor, const QPoint& anchorPoint);
    void rotateLeft();
    void rotateRight();
    void fitBounds(const QRectF& worldBounds);

    // --- Геттеры ---
    QTransform getWorldToScreenTransform() const;
    QTransform getScreenToWorldTransform() const;
    qreal getZoomFactor() const;

    // --- Сеттер/Геттер для Q_PROPERTY ---
    void setRotationAngle(qreal angle);
    qreal getRotationAngle() const;

signals:
    // Сигнал для ViewportPanelWidget, чтобы он перерисовался
    void updated();

private:
    QPointF m_panOffset{0.0, 0.0};
    double m_zoomFactor = 1.0;
    qreal m_rotationAngle = 0.0;

    int m_targetRotationStep = 0;
    QPropertyAnimation* m_rotationAnimation;
    QSize m_canvasSize;
};
