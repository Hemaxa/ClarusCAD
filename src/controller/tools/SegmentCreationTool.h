//SegmentCreationTool - инструмент создания объекта "Отрезок"

#pragma once

#include "BaseCreationTool.h"
#include "PointPrimitive.h"
#include "SnapManager.h"

#include <QColor>
#include <QPointF>

class SegmentPrimitive;

//наслдедуется от базового класса BaseCreationTool
class SegmentCreationTool : public BaseCreationTool
{
    Q_OBJECT

public:
    /**
     * @brief Конструктор.
     */
    explicit SegmentCreationTool(QObject* parent = nullptr);

    //переопределение методов действий мыши
    void onMousePress(QMouseEvent* event, Scene* scene, ViewportPanelWidget* viewport) override;
    void onMouseMove(QMouseEvent* event, Scene* scene, ViewportPanelWidget* viewport) override;
    void onMouseRelease(QMouseEvent* event, Scene* scene, ViewportPanelWidget* viewport) override;

    //переопределение метода очистки инструмента
    void reset() override;

    //переопределение метода для установки цвета
    void setColor(const QColor& color) override;

    //переопределение метода для установки типа линии
    void setLineType(LineType type) override;

    //переопределение метода для получения установленного цвета
    QColor getColor() const override;

    //переопределение вспомогательного метода для дополнительной геометрии
    void onPaint(QPainter& painter) override;

signals:
    /**
     * @brief Сигнал готовности данных отрезка.
     * @param segment Указатель на созданный примитив (может быть nullptr).
     * @param start Начальная точка.
     * @param end Конечная точка.
     * @param color Цвет.
     * @param lineType Тип линии.
     */
    void segmentDataReady(SegmentPrimitive* segment, const PointPrimitive& start, const PointPrimitive& end, const QColor& color, LineType lineType);

private:
    struct ResolvedSnap {
        QPointF position;
        SnapType type = SnapType::None;
    };

    //состояние ввода
    enum class State {
        Idle,
        WaitingForSecondPoint
    };

    State m_currentState; //текущее состояние ввода
    PointPrimitive m_firstPoint; //переменная хранения координат первой точки
    PointPrimitive m_currentMousePos; //переменная хранения текущей позиции мыши
    QColor m_currentColor = Qt::white; //переменная хранения цвета
    LineType m_currentLineType = LineType::SolidMain; //переменная хранения типа линии

    ResolvedSnap resolveSecondPoint(const QPointF& rawWorldPos, Qt::KeyboardModifiers modifiers,
                                    Scene* scene, ViewportPanelWidget* viewport) const;
    QPointF constrainToOrthoAxis(const QPointF& worldPos) const;
};
