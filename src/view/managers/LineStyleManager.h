#pragma once

#include "EnumManager.h"

#include <QObject>
#include <QPen>
#include <QMap>
#include <QVector>

class QPainter;

//Структура для хранения пользовательского стиля
struct CustomLineStyle {
    QString name;
    QVector<qreal> pattern; //Паттерн штриховки (например, 10px штрих, 5px пробел)
};

class LineStyleManager : public QObject
{
    Q_OBJECT

public:
    static LineStyleManager& instance();

    // Главный метод отрисовки линии.
    // Принимает int typeId для поддержки custom ID
    void drawLine(QPainter& painter, const QPointF& start, const QPointF& end,
                  int typeId, const QColor& color, bool isSelected = false) const;

    // Настройки базовой толщины
    void setBaseLineThickness(double thickness);
    double getBaseLineThickness() const;

    // Настройки параметров паттерна
    void setDashLength(double length);
    void setDashSpace(double space);

    // --- Методы для работы с пользовательскими стилями ---
    void addCustomStyle(int id, const QString& name, const QVector<qreal>& pattern);
    void removeCustomStyle(int id);
    QMap<int, CustomLineStyle> getCustomStyles() const;
    int generateNewId() const; //Генерирует уникальный ID > 1000

    //Получение имени и паттерна по ID (для UI)
    QString getStyleName(int id) const;
    QVector<qreal> getPattern(int id) const;

signals:
    void stylesChanged();

private:
    explicit LineStyleManager(QObject* parent = nullptr);

    double m_baseThickness = 2.0; // Пиксели
    double m_dashLength = 10.0;   // Текущая базовая длина штриха
    double m_dashSpace = 5.0;     // Текущее базовое расстояние

    // Константы, относительно которых происходит масштабирование
    // Соответствуют значениям в "Конструкторе стилей"
    const double DEFAULT_DASH_REF = 10.0;
    const double DEFAULT_SPACE_REF = 5.0;

    //Хранилище пользовательских стилей
    QMap<int, CustomLineStyle> m_customStyles;

    // Вспомогательные методы
    LineWeight getWeightForType(int typeId) const;
    QPen getPen(int typeId, const QColor& color, bool isSelected) const;

    // Генераторы путей для сложных линий
    void drawWaveLine(QPainter& painter, const QPointF& start, const QPointF& end, const QPen& pen) const;
    void drawZigzagLine(QPainter& painter, const QPointF& start, const QPointF& end, const QPen& pen) const;
};
