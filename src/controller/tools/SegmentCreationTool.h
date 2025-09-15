//SegmentCreationTool - инструмент создания отрезка

#pragma once

#include "BaseCreationTool.h"
#include "PointPrimitive.h"

//наслдедуется от базового класса BaseCreationTool
class SegmentCreationTool : public BaseCreationTool
{
    Q_OBJECT

public:
    //конструктор
    explicit SegmentCreationTool(QObject* parent = nullptr);

    //переопределение методов действий мыши
    void onMousePress(QMouseEvent* event, Scene* scene, ViewportPanelWidget* viewport) override;
    void onMouseMove(QMouseEvent* event, Scene* scene, ViewportPanelWidget* viewport) override;
    void onMouseRelease(QMouseEvent* event, Scene* scene, ViewportPanelWidget* viewport) override;

    //переопределение метода очистки инструмента
    void reset() override;

    //переопределение вспомогательного метода для дополнительной геометрии
    void onPaint(QPainter& painter) override;

signals:
    //сигнал, сообщающий об окончании ввода параметров и передающий координаты
    void segmentDataReady(const PointPrimitive& start, const PointPrimitive& end);

private:
    //состояние ввода
    enum class State {
        Idle,
        WaitingForSecondPoint
    };

    State m_currentState; //текущее состояние ввода
    PointPrimitive m_firstPoint; //переменная хранения координат первой точки
    PointPrimitive m_currentMousePos; //переменная хранения текущей позиции мыши
};
