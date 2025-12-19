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
    //виртуальные деструктор
    virtual ~BasePrimitive() = default;

    //виртуальные методы получения общей характеристики примитива
    virtual PrimitiveType getType() const { return PrimitiveType::Generic; }
    virtual QString getTypeName() const { return "Примитив"; }

    //виртуальные методы задания и получения имени примитива
    virtual void setName(const QString& name) { m_name = name; }
    virtual QString getName() const { return m_name; }

    //виртуальные методы задания и получения цвета примитива
    virtual void setColor(const QColor& color) { m_color = color; }
    virtual QColor getColor() const { return m_color; }

    //виртуальные методы задания и получения типа линии примитива
    virtual void setLineType(int typeId) { m_lineTypeId = typeId; }
    virtual int getLineType() const { return m_lineTypeId; }

    //перегрузка метода задания типа линии для пользовательских стилей (enum)
    void setLineType(LineType type) { m_lineTypeId = static_cast<int>(type); }

    // --- НОВЫЕ МЕТОДЫ SMART MODEL ---

    //1. Отрисовка (примитив знает как рисовать себя через LineStyleManager)
    //isSelected - флаг выделения (рисует подсветку)
    virtual void draw(QPainter& painter, bool isSelected) const = 0;

    //2. Габариты (Axis Aligned Bounding Box)
    virtual QRectF getBoundingBox() const = 0;

    //3. Проверка попадания кликом (для удаления и точечного выделения)
    virtual bool hitTest(const QPointF& point, double tolerance) const = 0;

    //4. Проверка пересечения с рамкой (выделение Crossing / Зеленая рамка)
    virtual bool intersects(const QRectF& rect) const = 0;

    //5. Проверка нахождения внутри рамки (выделение Window / Синяя рамка)
    virtual bool inside(const QRectF& rect) const = 0;

    //6. Точки привязки для SnapManager (концы, середины, центры)
    virtual QVector<QPointF> getSnapPoints() const = 0;

private:
    QString m_name; //имя примитива
    QColor m_color = Qt::white; //цвет примитива
    int m_lineTypeId = static_cast<int>(LineType::SolidMain); //тип линии примитва
};
