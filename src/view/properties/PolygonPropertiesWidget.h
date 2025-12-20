//PolygonPropertiesWidget - виджет свойств примитива "Многоугольник"

#pragma once

#include "BasePropertiesWidget.h"
#include "PointPrimitive.h"
#include "EnumManager.h"

#include <QLabel>

class QSpinBox;
class QComboBox;
class PolygonPrimitive;

//наследуется от базового класса BasePropertiesWidget
class PolygonPropertiesWidget : public BasePropertiesWidget
{
    Q_OBJECT

public:
    /**
     * @brief Конструктор виджета свойств многоугольника.
     */
    explicit PolygonPropertiesWidget(QWidget* parent = nullptr);

    /**
     * @brief Установить редактируемые примитивы.
     */
    void setPrimitives(const QList<BasePrimitive*>& primitives) override;

signals:
    /**
     * @brief Сигнал применения данных (для создания или обновления).
     */
    void propertiesApplied(PolygonPrimitive* polygon, int sides, PolygonCreationMode type, 
                           const QColor& color, LineType lineType);
    
    /**
     * @brief Сигнал изменения количества сторон (для инструмента).
     */
    void sidesChanged(int sides);

    /**
     * @brief Сигнал изменения типа построения (для инструмента).
     */
    void polygonTypeChanged(PolygonCreationMode type);

private slots:
    void onApplyButtonClicked();
    void onSidesChanged(int value);
    void onTypeChanged(int index);

private:
    void updateFieldValues() override;

    PolygonPrimitive* m_currentPolygon = nullptr; ///< Текущий редактируемый многоугольник

    QSpinBox* m_sidesSpinBox;      ///< Количество углов
    QComboBox* m_typeComboBox;     ///< Тип (вписанный/описанный)
};
