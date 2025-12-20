//BasePrimitive - базовый класс для всех хранилищ данных объектов

#pragma once

#include "EnumManager.h"

#include <QColor> //для хранения цвета объекта
#include <QString> //для хранения имени объекта
#include <QObject>
#include <QRectF>
#include <QPainter>
#include <QVector>

class BasePrimitive
{

public:
    /**
     * @brief Виртуальный деструктор.
     */
    virtual ~BasePrimitive() = default;

    /**
     * @brief Получить тип примитива.
     * @return Тип примитива из перечисления PrimitiveType.
     */
    virtual PrimitiveType getType() const { return PrimitiveType::Generic; }

    /**
     * @brief Получить строковое название типа примитива.
     * @return Название типа (например, "Отрезок", "Окружность").
     */
    virtual QString getTypeName() const { return "Примитив"; }

    /**
     * @brief Установить имя примитива.
     * @param name Новое имя.
     */
    virtual void setName(const QString& name) { m_name = name; }

    /**
     * @brief Получить имя примитива.
     * @return Имя примитива.
     */
    virtual QString getName() const { return m_name; }

    /**
     * @brief Установить цвет примитива.
     * @param color Новый цвет.
     */
    virtual void setColor(const QColor& color) { m_color = color; }

    /**
     * @brief Получить текущий цвет примитива.
     * @return Цвет примитива.
     */
    virtual QColor getColor() const { return m_color; }

    /**
     * @brief Установить ID типа линии.
     * @param typeId ID типа линии (из LineType или пользовательский).
     */
    virtual void setLineType(int typeId) { m_lineTypeId = typeId; }

    /**
     * @brief Получить ID типа линии.
     * @return ID типа линии.
     */
    virtual int getLineType() const { return m_lineTypeId; }

    /**
     * @brief Установить тип линии через enum LineType.
     * @param type Тип линии.
     */
    void setLineType(LineType type) { m_lineTypeId = static_cast<int>(type); }

    // --- НОВЫЕ МЕТОДЫ SMART MODEL ---

    /**
     * @brief Отрисовка примитива.
     * @param painter Объект QPainter для рисования.
     * @param isSelected Флаг выделения (если true, рисуется с подсветкой).
     */
    virtual void draw(QPainter& painter, bool isSelected) const = 0;

    /**
     * @brief Получить ограничивающий прямоугольник (AABB).
     * @return Прямоугольник QRectF, охватывающий примитив.
     */
    virtual QRectF getBoundingBox() const = 0;

    /**
     * @brief Проверка попадания точки в примитив.
     * @param point Точка клика.
     * @param tolerance Допуск (расстояние), в пределах которого считается попадание.
     * @return true, если точка попадает в примитив, иначе false.
     */
    virtual bool hitTest(const QPointF& point, double tolerance) const = 0;

    /**
     * @brief Проверка пересечения примитива с прямоугольной областью.
     * Используется для выделения рамкой (Crossing selection).
     * @param rect Прямоугольная область.
     * @return true, если примитив пересекается с областью или находится внутри неё.
     */
    virtual bool intersects(const QRectF& rect) const = 0;

    /**
     * @brief Проверка полного нахождения примитива внутри прямоугольной области.
     * Используется для выделения рамкой (Window selection).
     * @param rect Прямоугольная область.
     * @return true, если примитив полностью внутри области.
     */
    virtual bool inside(const QRectF& rect) const = 0;

    /**
     * @brief Получить ключевые точки привязки (Snap Points).
     * Например: концы отрезка, середина, центр окружности, квадранты.
     * @return Вектор точек привязки.
     */
    virtual QVector<QPointF> getSnapPoints() const = 0;

private:
    QString m_name;                                         ///< Имя примитива
    QColor m_color = Qt::white;                             ///< Цвет примитива
    int m_lineTypeId = static_cast<int>(LineType::SolidMain); ///< Тип линии примитива
};
