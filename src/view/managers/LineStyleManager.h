#pragma once

#include "EnumManager.h"

#include <QObject>
#include <QPen>
#include <QMap>
#include <QVector>

class QPainter;

struct CustomLineStyle {
    QString name;
    QVector<qreal> pattern;
};

class LineStyleManager : public QObject
{
    Q_OBJECT

public:
    static LineStyleManager& instance();

    void drawLine(QPainter& painter, const QPointF& start, const QPointF& end,
                  int typeId, const QColor& color, bool isSelected = false) const;

    void drawEllipse(QPainter& painter, const QPointF& center, double rx, double ry,
                     int typeId, const QColor& color, bool isSelected = false) const;

    // --- ЭТОТ МЕТОД ТЕПЕРЬ PUBLIC ---
    QPen getPen(int typeId, const QColor& color, bool isSelected) const;

    void setBaseLineThickness(double thickness);
    double getBaseLineThickness() const;

    void setDashLength(double length);
    void setDashSpace(double space);

    void addCustomStyle(int id, const QString& name, const QVector<qreal>& pattern);
    void removeCustomStyle(int id);
    QMap<int, CustomLineStyle> getCustomStyles() const;
    int generateNewId() const;

    QString getStyleName(int id) const;
    QVector<qreal> getPattern(int id) const;

signals:
    void stylesChanged();

private:
    explicit LineStyleManager(QObject* parent = nullptr);

    double m_baseThickness = 2.0;
    double m_dashLength = 10.0;
    double m_dashSpace = 5.0;

    const double DEFAULT_DASH_REF = 10.0;
    const double DEFAULT_SPACE_REF = 5.0;

    QMap<int, CustomLineStyle> m_customStyles;

    LineWeight getWeightForType(int typeId) const;

    // Вспомогательные методы отрисовки спец. линий
    void drawWaveLine(QPainter& painter, const QPointF& start, const QPointF& end, const QPen& pen) const;
    void drawZigzagLine(QPainter& painter, const QPointF& start, const QPointF& end, const QPen& pen) const;
};
