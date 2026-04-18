// BaseDimensionPrimitive.h - базовый класс для всех размерных примитивов

#pragma once

#include "../../BasePrimitive.h"
#include "../../DimensionStyle.h"
#include <QString>
#include <QPainter>
#include <QRectF>
#include <QPointF>
#include <QVector>

/**
 * @brief Базовый класс для всех размерных примитивов
 */
class BaseDimensionPrimitive : public BasePrimitive {
public:
    BaseDimensionPrimitive() = default;
    virtual ~BaseDimensionPrimitive() = default;

    /**
     * @brief Установить стиль размера
     */
    void setStyle(const DimensionStyle& style) { m_style = style; }

    /**
     * @brief Получить текущий стиль размера
     */
    DimensionStyle getStyle() const { return m_style; }

    /**
     * @brief Установить вычисленное значение размера
     */
    void setMeasuredValue(double value) { m_measuredValue = value; }

    /**
     * @brief Получить вычисленное значение размера
     */
    double getMeasuredValue() const { return m_measuredValue; }

    /**
     * @brief Установить переопределенный текст
     * @param text Произвольный текст (если пусто — выводится m_measuredValue)
     */
    void setCustomText(const QString& text) { m_customText = text; }

    /**
     * @brief Получить переопределенный текст
     */
    QString getCustomText() const { return m_customText; }

    /**
     * @brief Пересчитать значение измеренного размера
     */
    virtual void recalculateValue() = 0;

    // --- Заглушки для чисто виртуальных методов BasePrimitive ---

    virtual void draw(QPainter& painter, bool isSelected) const override {}
    virtual QRectF getBoundingBox() const override { return QRectF(); }
    virtual bool hitTest(const QPointF& point, double tolerance) const override { return false; }
    virtual bool intersects(const QRectF& rect) const override { return false; }
    virtual bool inside(const QRectF& rect) const override { return false; }
    virtual QVector<QPointF> getSnapPoints() const override { return QVector<QPointF>(); }

protected:
    DimensionStyle m_style;           ///< Настройки стиля размера
    double m_measuredValue = 0.0;     ///< Вычисленное значение
    QString m_customText = "";        ///< Переопределенный текст
};
