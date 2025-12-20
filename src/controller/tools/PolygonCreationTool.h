//PolygonCreationTool - инструмент создания многоугольника

#pragma once

#include "BaseCreationTool.h"
#include "PointPrimitive.h"

class PolygonPrimitive;

class PolygonCreationTool : public BaseCreationTool
{
    Q_OBJECT

public:
public:
    /**
     * @brief Конструктор.
     */
    explicit PolygonCreationTool(QObject* parent = nullptr);

    // Переопределение методов мыши
    void onMousePress(QMouseEvent* event, Scene* scene, ViewportPanelWidget* viewport) override;
    void onMouseMove(QMouseEvent* event, Scene* scene, ViewportPanelWidget* viewport) override;
    void onMouseRelease(QMouseEvent* event, Scene* scene, ViewportPanelWidget* viewport) override;

    void reset() override;
    void onPaint(QPainter& painter) override;

    void setColor(const QColor& color) override;
    void setLineType(LineType type) override;
    QColor getColor() const override;

    // Настройки построения
    
    /**
     * @brief Установить количество сторон многоугольника.
     */
    void setSides(int sides);

    /**
     * @brief Получить количество сторон.
     */
    int getSides() const;
    
    /**
     * @brief Установить тип многоугольника (вписанный/описанный).
     */
    void setPolygonType(PolygonCreationMode mode);

    /**
     * @brief Получить текущий тип многоугольника.
     */
    PolygonCreationMode getPolygonType() const;

signals:
    /**
     * @brief Сигнал завершения создания многоугольника.
     * @param polygon Указатель на созданный примитив.
     */
    void polygonDataReady(PolygonPrimitive* polygon);

private:
    int m_step = 0; // 0 - ожидание центра, 1 - ожидание радиуса
    PointPrimitive m_center;
    PointPrimitive m_currentMousePos;
    
    QColor m_currentColor = Qt::white;
    LineType m_currentLineType = LineType::SolidMain;
    
    int m_sides = 6; // По умолчанию 6-угольник
    PolygonCreationMode m_polygonMode = PolygonCreationMode::Inscribed;
};
