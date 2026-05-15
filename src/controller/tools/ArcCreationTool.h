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
    /**
     * @brief Конструктор инструмента создания дуги.
     */
    explicit ArcCreationTool(QObject* parent = nullptr);

    /**
     * @brief Установить режим создания дуги.
     * @param mode Режим (например, Центр-Старт-Конец или 3 точки).
     */
    void setCreationMode(ArcCreationMode mode);

    void onMousePress(QMouseEvent* e, Scene* s, ViewportPanelWidget* v) override;
    void onMouseMove(QMouseEvent* e, Scene* s, ViewportPanelWidget* v) override;
    void onMouseRelease(QMouseEvent* e, Scene* s, ViewportPanelWidget* v) override;
    void onPaint(QPainter& painter) override;
    void reset() override;

    void setColor(const QColor& color) override { m_currentColor = color; }
    void setLineType(LineType type) override { m_currentLineType = type; }
    QColor getColor() const override { return m_currentColor; }
    ArcCreationMode getCreationMode() const { return m_mode; }
    PointPrimitive getFirstPoint() const { return m_p1; }
    PointPrimitive getSecondPoint() const { return m_p2; }
    PointPrimitive getThirdPoint() const { return m_p3; }

signals:
    /**
     * @brief Сигнал готовности новой дуги.
     * @param arc Указатель на созданный примитив дуги.
     */
    void arcDataReady(ArcPrimitive* arc);

private:
    ArcCreationMode m_mode = ArcCreationMode::CenterStartEnd;
    int m_step = 0;

    PointPrimitive m_p1;
    PointPrimitive m_p2;
    PointPrimitive m_p3;
    PointPrimitive m_currentPos;

    QColor m_currentColor = Qt::white;
    LineType m_currentLineType = LineType::SolidMain;
};
