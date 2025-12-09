//CircleCreationTool - инструмент создания объекта "Окружность" с поддержкой разных режимов

#pragma once

#include "BaseCreationTool.h"
#include "PointPrimitive.h"

#include <QColor>

class CirclePrimitive;

class CircleCreationTool : public BaseCreationTool
{
    Q_OBJECT

public:
    explicit CircleCreationTool(QObject* parent = nullptr);

    // Установка режима построения
    void setCreationMode(CircleCreationMode mode);

    // Переопределение методов действий мыши
    void onMousePress(QMouseEvent* event, Scene* scene, ViewportPanelWidget* viewport) override;
    void onMouseMove(QMouseEvent* event, Scene* scene, ViewportPanelWidget* viewport) override;
    void onMouseRelease(QMouseEvent* event, Scene* scene, ViewportPanelWidget* viewport) override;

    void reset() override;
    void onPaint(QPainter& painter) override;

    void setColor(const QColor& color) override;
    void setLineType(LineType type) override;
    QColor getColor() const override;

signals:
    // Сигнал готовности данных (передает созданный примитив)
    void circleDataReady(CirclePrimitive* circle);

private:
    CircleCreationMode m_mode = CircleCreationMode::CenterRadius;
    int m_step = 0; // Текущий шаг ввода (0, 1, 2...)

    PointPrimitive m_p1; // Первая точка (центр или точка на окружности)
    PointPrimitive m_p2; // Вторая точка
    PointPrimitive m_p3; // Третья точка

    PointPrimitive m_currentMousePos;
    QColor m_currentColor = Qt::white;
    LineType m_currentLineType = LineType::SolidMain;

    // Вспомогательные методы
    bool getCircleFrom3Points(const PointPrimitive& p1, const PointPrimitive& p2, const PointPrimitive& p3, PointPrimitive& center, double& radius);
};
