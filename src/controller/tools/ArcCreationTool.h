//ArcCreationTool - инструмент создания дуги

#pragma once
#include "BaseCreationTool.h"
#include "PointPrimitive.h"
#include <QColor>

class ArcPrimitive;

class ArcCreationTool : public BaseCreationTool
{
    Q_OBJECT
public:
    explicit ArcCreationTool(QObject* parent = nullptr);

    void setCreationMode(ArcCreationMode mode);

    void onMousePress(QMouseEvent* e, Scene* s, ViewportPanelWidget* v) override;
    void onMouseMove(QMouseEvent* e, Scene* s, ViewportPanelWidget* v) override;
    void onMouseRelease(QMouseEvent* e, Scene* s, ViewportPanelWidget* v) override;
    void onPaint(QPainter& painter) override;
    void reset() override;

    void setColor(const QColor& color) override { m_currentColor = color; }
    void setLineType(LineType type) override { m_currentLineType = type; }

signals:
    // Передаем готовый объект, так как расчеты дуги сложны
    void arcDataReady(ArcPrimitive* arc);

private:
    ArcCreationMode m_mode = ArcCreationMode::CenterStartEnd;
    int m_step = 0;

    PointPrimitive m_p1;
    PointPrimitive m_p2;
    PointPrimitive m_currentPos;

    QColor m_currentColor = Qt::white;
    LineType m_currentLineType = LineType::SolidMain;
};
