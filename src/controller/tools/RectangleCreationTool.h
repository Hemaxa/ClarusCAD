#pragma once
#include "BaseCreationTool.h"
#include "PointPrimitive.h"

class RectangleCreationTool : public BaseCreationTool {
    Q_OBJECT
public:
    explicit RectangleCreationTool(QObject* parent = nullptr);

    void setCreationMode(RectangleCreationMode mode);

    void onMousePress(QMouseEvent* e, Scene* s, ViewportPanelWidget* v) override;
    void onMouseMove(QMouseEvent* e, Scene* s, ViewportPanelWidget* v) override;
    void onMouseRelease(QMouseEvent* e, Scene* s, ViewportPanelWidget* v) override;
    void onPaint(QPainter& painter) override;
    void reset() override;

    void setColor(const QColor& color) override { m_currentColor = color; }
    void setLineType(LineType type) override { m_currentLineType = type; }

signals:
    // Сигнал передает параметры для создания прямоугольника в MainWindow
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
