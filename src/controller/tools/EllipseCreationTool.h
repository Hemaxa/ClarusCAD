//EllipseCreationTool - инструмент создания эллипса

#pragma once
#include "BaseCreationTool.h"
#include "PointPrimitive.h"

class EllipseCreationTool : public BaseCreationTool {
    Q_OBJECT
public:
    /**
     * @brief Конструктор инструмента создания эллипса.
     */
    explicit EllipseCreationTool(QObject* parent = nullptr);

    void onMousePress(QMouseEvent* e, Scene* s, ViewportPanelWidget* v) override;
    void onMouseMove(QMouseEvent* e, Scene* s, ViewportPanelWidget* v) override;
    void onMouseRelease(QMouseEvent* e, Scene* s, ViewportPanelWidget* v) override;
    void onPaint(QPainter& p) override;
    void reset() override;

    void setColor(const QColor& c) override { m_currentColor = c; }
    void setLineType(LineType t) override { m_currentLineType = t; }
    QColor getColor() const override { return m_currentColor; }

signals:
    /**
     * @brief Сигнал готовности параметров нового эллипса.
     * @param center Центр.
     * @param rx Радиус X.
     * @param ry Радиус Y.
     * @param rot Угол поворота.
     */
    void ellipseDataReady(const PointPrimitive& center, double rx, double ry, double rot);

private:
    int m_step = 0;
    PointPrimitive m_center;
    PointPrimitive m_axisPoint1;
    PointPrimitive m_currentPos;

    QColor m_currentColor = Qt::white;
    LineType m_currentLineType = LineType::SolidMain;
};
