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
    /**
     * @brief Установить размер холста (размер виджета в пикселях).
     */
    void setCanvasSize(const QSize& size);

    /**
     * @brief Панорамирование в координатах экрана.
     * @param screenDelta Сдвиг в пикселях.
     */
    void pan(const QPointF& screenDelta);

    /**
     * @brief Панорамирование в координатах мира.
     * @param worldDelta Сдвиг в единицах мира.
     */
    void panWorld(const QPointF& worldDelta);

    /**
     * @brief Приближение/отдаление (Zoom).
     * @param factor Коэффициент масштабирования ( > 1 для зума, < 1 для отдаления).
     * @param anchorPoint Точка якоря (обычно позиция курсора) в координатах экрана.
     */
    void applyZoom(double factor, const QPoint& anchorPoint);

    /**
     * @brief Повернуть камеру влево на 90 градусов.
     */
    void rotateLeft();

    /**
     * @brief Повернуть камеру вправо на 90 градусов.
     */
    void rotateRight();

    /**
     * @brief Вписать границы мира в экран (Zoom Extents).
     * @param worldBounds Прямоугольная область в мире, которую надо показать.
     */
    void fitBounds(const QRectF& worldBounds);

    // --- Геттеры ---
    /**
     * @brief Получить матрицу трансформации из Мира в Экран.
     */
    QTransform getWorldToScreenTransform() const;

    /**
     * @brief Получить матрицу трансформации из Экрана в Мир.
     */
    QTransform getScreenToWorldTransform() const;

    /**
     * @brief Получить текущий коэффициент масштаба.
     */
    qreal getZoomFactor() const;

    // --- Сеттер/Геттер для Q_PROPERTY ---
    void setRotationAngle(qreal angle);
    qreal getRotationAngle() const;

signals:
    /**
     * @brief Сигнал об изменении параметров камеры.
     * Требует перерисовки ViewportPanelWidget.
     */
    void updated();

private:
    QPointF m_panOffset{0.0, 0.0};
    double m_zoomFactor = 1.0;
    qreal m_rotationAngle = 0.0;

    int m_targetRotationStep = 0;
    QPropertyAnimation* m_rotationAnimation;
    QSize m_canvasSize;
};
