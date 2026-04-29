//RectangleCreationTool - инструмент создания прямоугольника

#pragma once
#include "BaseCreationTool.h"
#include "PointPrimitive.h"

class RectangleCreationTool : public BaseCreationTool {
    Q_OBJECT
public:
public:
    /**
     * @brief Конструктор инструмента.
     */
    explicit RectangleCreationTool(QObject* parent = nullptr);

    /**
     * @brief Установить режим создания прямоугольника.
     * @param mode Режим (например, по двум точкам).
     */
    void setCreationMode(RectangleCreationMode mode);

    void onMousePress(QMouseEvent* e, Scene* s, ViewportPanelWidget* v) override;
    void onMouseMove(QMouseEvent* e, Scene* s, ViewportPanelWidget* v) override;
    void onMouseRelease(QMouseEvent* e, Scene* s, ViewportPanelWidget* v) override;
    void onPaint(QPainter& painter) override;
    void reset() override;

    void setColor(const QColor& color) override { m_currentColor = color; }
    void setLineType(LineType type) override { m_currentLineType = type; }
    QColor getColor() const override { return m_currentColor; }

signals:
    /**
     * @brief Сигнал готовности данных прямоугольника.
     * @param center Центр прямоугольника.
     * @param width Ширина.
     * @param height Высота.
     * @param rotation Угол поворота.
     */
    void rectangleDataReady(const PointPrimitive& center, double width, double height, double rotation);

private:
    RectangleCreationMode m_mode = RectangleCreationMode::TwoPoints;
    int m_step = 0;

    PointPrimitive m_p1; // Первая точка
    PointPrimitive m_p2; // Вторая точка (для режима 3 точек)
    PointPrimitive m_currentPos; // Текущая позиция мыши

    QColor m_currentColor = Qt::white;
    LineType m_currentLineType = LineType::SolidMain;
};
