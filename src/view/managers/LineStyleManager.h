//LineStyleManager - менеджер стилей линий и их отрисовки

#pragma once

#include "EnumManager.h"

#include <QObject>
#include <QPen>
#include <QMap>
#include <QVector>

class QPainter;

// Структура пользовательского стиля линии
/**
 * @brief Структура пользовательского стиля линии.
 */
struct CustomLineStyle {
    QString name;               ///< Имя стиля
    QVector<qreal> pattern;     ///< Шаблон штрихов и пробелов
    double thickness = -1.0;    ///< Толщина (-1.0 означает использование глобальной толщины)
};

class LineStyleManager : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief Получить доступ к синглтону.
     */
    static LineStyleManager& instance();

    /**
     * @brief Отрисовка линии.
     * @param painter QPainter.
     * @param start Начало.
     * @param end Конец.
     * @param typeId ID стиля линии.
     * @param color Цвет.
     * @param isSelected Выделена ли.
     */
    void drawLine(QPainter& painter, const QPointF& start, const QPointF& end,
                  int typeId, const QColor& color, bool isSelected = false) const;

    /**
     * @brief Отрисовка эллипса/окружности.
     */
    void drawEllipse(QPainter& painter, const QPointF& center, double rx, double ry,
                     int typeId, const QColor& color, bool isSelected = false) const;

    /**
     * @brief Отрисовка дуги с поддержкой спецэффектов (волна/зигзаг).
     */
    void drawArc(QPainter& painter, const QPointF& center, double radius,
                 double startAngle, double spanAngle,
                 int typeId, const QColor& color, bool isSelected = false) const;

    /**
     * @brief Отрисовка произвольного пути (для полигонов, прямоугольников).
     */
    void drawPath(QPainter& painter, const QPainterPath& path,
                  int typeId, const QColor& color, bool isSelected = false) const;

    /**
     * @brief Получить перо (QPen) для отрисовки.
     */
    QPen getPen(int typeId, const QColor& color, bool isSelected) const;

    // Версии с поддержкой непрерывной фазы (для криволинейных примитивов)
    double drawWaveLineWithPhase(QPainter& painter, const QPointF& start, const QPointF& end, 
                                  const QPen& pen, double startPhase) const;
    double drawZigzagLineWithPhase(QPainter& painter, const QPointF& start, const QPointF& end, 
                                    const QPen& pen, double startPhase) const;

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
