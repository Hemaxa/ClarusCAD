//CirclePropertiesWidget - виджет свойств примитива "Окружность"

#pragma once

#include "BasePropertiesWidget.h"
#include "PointPrimitive.h"

#include <QLabel>
#include <QLineEdit>
#include <QStackedWidget>

                                       class CirclePrimitive;

class CirclePropertiesWidget : public BasePropertiesWidget
{
    Q_OBJECT

public:
    explicit CirclePropertiesWidget(QWidget* parent = nullptr);

    void setPrimitives(const QList<BasePrimitive*>& primitives) override;

signals:
    // Сигнал для обновления существующей окружности
    void propertiesApplied(CirclePrimitive* circle, const PointPrimitive& center, double radius, const QColor& color, LineType lineType);

private slots:
    void onApplyButtonClicked();

    // Слоты для пересчета значений внутри режимов
    void onCartesianRadiusChanged(const QString& text);
    void onCartesianDiameterChanged(const QString& text);

    void onModeChanged(int index);

private:
    void updateFieldValues() override;

    // Вспомогательный метод для расчета окружности по 3 точкам
    bool getCircleFrom3Points(const PointPrimitive& p1, const PointPrimitive& p2, const PointPrimitive& p3, PointPrimitive& outCenter, double& outRadius);

    CirclePrimitive* m_currentCircle = nullptr;

    // Выбор режима редактирования
    QComboBox* m_modeComboBox;
    QStackedWidget* m_modeStack;

    // --- Режим 1: Центр + Радиус ---
    QWidget* m_pageCenterRadius;
    QLineEdit* m_crCenterX;
    QLineEdit* m_crCenterY;
    QLineEdit* m_crRadius;

    // --- Режим 2: Центр + Диаметр ---
    QWidget* m_pageCenterDiameter;
    QLineEdit* m_cdCenterX;
    QLineEdit* m_cdCenterY;
    QLineEdit* m_cdDiameter;

    // --- Режим 3: Две точки (Диаметр) ---
    QWidget* m_pageTwoPoints;
    QLineEdit* m_tpX1;
    QLineEdit* m_tpY1;
    QLineEdit* m_tpX2;
    QLineEdit* m_tpY2;

    // --- Режим 4: Три точки ---
    QWidget* m_pageThreePoints;
    QLineEdit* m_thpX1;
    QLineEdit* m_thpY1;
    QLineEdit* m_thpX2;
    QLineEdit* m_thpY2;
    QLineEdit* m_thpX3;
    QLineEdit* m_thpY3;

    // Полярные координаты (пока оставим упрощенно, т.к. режимы выше перекрывают основные кейсы)
    // Для чистоты UI будем показывать/скрывать их или интегрировать.
    // В текущей реализации сделаем режимы главными для Декартовой системы.
};
