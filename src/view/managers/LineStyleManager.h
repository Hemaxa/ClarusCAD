#pragma once

#include "EnumManager.h"

#include <QObject>
#include <QPen>
#include <QMap>
#include <QVector>

class QPainter;

// Структура пользовательского стиля линии
struct CustomLineStyle {
    QString name;
    QVector<qreal> pattern;
    double thickness = -1.0;  // -1 означает использование глобальной толщины
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

    // Получение пера для отрисовки
    QPen getPen(int typeId, const QColor& color, bool isSelected) const;

    // --- Базовые параметры линий ---
    void setBaseLineThickness(double thickness);
    double getBaseLineThickness() const;

    void setDashLength(double length);
    double getDashLength() const;
    void setDashSpace(double space);
    double getDashSpace() const;

    // --- Параметры волнистой линии ---
    void setWaveAmplitude(double val);
    double getWaveAmplitude() const;
    void setWavePeriod(double val);
    double getWavePeriod() const;

    // --- Параметры линии с изломами ---
    void setKinkAmplitude(double val);
    double getKinkAmplitude() const;
    void setKinkLength(double val);
    double getKinkLength() const;
    void setKinkStraight(double val);
    double getKinkStraight() const;

    // --- Пользовательские стили ---
    void addCustomStyle(int id, const QString& name, const QVector<qreal>& pattern, double thickness = -1.0);
    void updateCustomStyle(int id, const QString& name, const QVector<qreal>& pattern, double thickness = -1.0);
    void removeCustomStyle(int id);
    QMap<int, CustomLineStyle> getCustomStyles() const;
    int generateNewId() const;

    QString getStyleName(int id) const;
    QVector<qreal> getPattern(int id) const;

signals:
    void stylesChanged();

private:
    explicit LineStyleManager(QObject* parent = nullptr);

    // Базовые параметры
    double m_baseThickness = 2.0;
    double m_dashLength = 10.0;
    double m_dashSpace = 5.0;

    // Параметры волнистой линии
    double m_waveAmplitude = 2.0;
    double m_wavePeriod = 10.0;

    // Параметры линии с изломами (зигзаг)
    double m_kinkAmplitude = 3.0;
    double m_kinkLength = 5.0;
    double m_kinkStraight = 10.0;

    const double DEFAULT_DASH_REF = 10.0;
    const double DEFAULT_SPACE_REF = 5.0;

    QMap<int, CustomLineStyle> m_customStyles;

    LineWeight getWeightForType(int typeId) const;

    // Вспомогательные методы отрисовки спец. линий
    void drawWaveLine(QPainter& painter, const QPointF& start, const QPointF& end, const QPen& pen) const;
    void drawZigzagLine(QPainter& painter, const QPointF& start, const QPointF& end, const QPen& pen) const;
};
