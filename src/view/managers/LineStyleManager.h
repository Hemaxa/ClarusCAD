#pragma once

#include "EnumManager.h"

#include <QObject>
#include <QPen>

class QPainter;

class LineStyleManager : public QObject
{
    Q_OBJECT

public:
    static LineStyleManager& instance();

    // Главный метод отрисовки линии.
    // Сам выбирает стратегию (QPen или QPainterPath) в зависимости от LineType
    void drawLine(QPainter& painter, const QPointF& start, const QPointF& end,
                  LineType type, const QColor& color, bool isSelected = false) const;

    // Настройки
    void setBaseLineThickness(double thickness);
    double getBaseLineThickness() const;

signals:
    void stylesChanged();

private:
    explicit LineStyleManager(QObject* parent = nullptr);

    double m_baseThickness = 2.0; // Пиксели
    double m_dashScale = 1.0;

    // Вспомогательные методы
    LineWeight getWeightForType(LineType type) const;
    QVector<qreal> getDashPattern(LineType type) const;
    QPen getPen(LineType type, const QColor& color, bool isSelected) const;

    // Генераторы путей для сложных линий
    void drawWaveLine(QPainter& painter, const QPointF& start, const QPointF& end, const QPen& pen) const;
    void drawZigzagLine(QPainter& painter, const QPointF& start, const QPointF& end, const QPen& pen) const;
};
