//SegmentCreationTool - инструмент создания объекта "Отрезок"

#pragma once

#include "BaseCreationTool.h"
#include "PointPrimitive.h"

#include <QColor>

class SegmentPrimitive;

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

    //переопределение метода для установки цвета
    void setColor(const QColor& color) override;

    //переопределение метода для получения установленного цвета
    QColor getColor() const override;

    //переопределение вспомогательного метода для дополнительной геометрии
    void onPaint(QPainter& painter) override;

signals:
    //сигнал, сообщающий об окончании ввода параметров и передающий координаты
    void segmentDataReady(SegmentPrimitive* segment, const PointPrimitive& start, const PointPrimitive& end, const QColor& color);

private:
    //состояние ввода
    enum class State {
        Idle,
        WaitingForSecondPoint
    };

    State m_currentState; //текущее состояние ввода
    PointPrimitive m_firstPoint; //переменная хранения координат первой точки
    PointPrimitive m_currentMousePos; //переменная хранения текущей позиции мыши
    QColor m_currentColor = Qt::white; //переменная хранения цвета
};
