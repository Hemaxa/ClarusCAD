//SplineCreationTool - инструмент создания сплайна по контрольным точкам

#pragma once

#include "BaseCreationTool.h"
#include <QVector>
#include <QPointF>

class SplinePrimitive;

class SplineCreationTool : public BaseCreationTool
{
    Q_OBJECT

public:
    explicit SplineCreationTool(QObject* parent = nullptr);

    // Переопределение методов мыши
    void onMousePress(QMouseEvent* event, Scene* scene, ViewportPanelWidget* viewport) override;
    void onMouseMove(QMouseEvent* event, Scene* scene, ViewportPanelWidget* viewport) override;
    void onMouseRelease(QMouseEvent* event, Scene* scene, ViewportPanelWidget* viewport) override;
    
    // Обработка клавиш (Enter для завершения, Escape для отмены)
    void onKeyPress(QKeyEvent* event, Scene* scene, ViewportPanelWidget* viewport) override;

    void reset() override;
    void onPaint(QPainter& painter) override;

    void setColor(const QColor& color) override;
    void setLineType(LineType type) override;
    QColor getColor() const override;

    // Завершить построение сплайна
    void finishSpline();

    // Настройка замкнутости сплайна
    void setClosed(bool closed) { m_closed = closed; }
    bool isClosed() const { return m_closed; }

signals:
    void splineDataReady(SplinePrimitive* spline);

private:
    QVector<QPointF> m_controlPoints;
    QPointF m_currentMousePos;
    
    QColor m_currentColor = Qt::white;
    LineType m_currentLineType = LineType::SolidMain;
    bool m_closed = false;
};
