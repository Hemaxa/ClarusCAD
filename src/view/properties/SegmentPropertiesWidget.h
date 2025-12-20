//SegmentPropertiesWidget - виджет свойств примитива "Отрезок"

#pragma once

#include "BasePropertiesWidget.h"
#include "PointPrimitive.h"

#include <QLabel>

class QLineEdit;
class SegmentPrimitive;

//наслдедуется от базового класса BasePropertiesWidget
class SegmentPropertiesWidget : public BasePropertiesWidget
{
    Q_OBJECT

public:
    /**
     * @brief Конструктор виджета свойств отрезка.
     */
    explicit SegmentPropertiesWidget(QWidget* parent = nullptr);

    /**
     * @brief Установить редактируемые примитивы.
     */
    void setPrimitives(const QList<BasePrimitive*>& primitives) override;

signals:
    /**
     * @brief Сигнал применения данных (для создания или обновления).
     */
    void propertiesApplied(SegmentPrimitive* segment, const PointPrimitive& start, const PointPrimitive& end, const QColor& color, LineType lineType);

private slots:
    void onApplyButtonClicked();

private:
    void updateFieldValues() override;

    SegmentPrimitive* m_currentSegment = nullptr; ///< Текущий редактируемый отрезок (главный из выделенных)

    // Поля объекта "Отрезок"
    // Декартова система координат
    QLineEdit* m_startXEdit;
    QLineEdit* m_startYEdit;
    QLineEdit* m_endXEdit;
    QLineEdit* m_endYEdit;

    // Полярная система координат
    QLineEdit* m_startRadiusEdit;
    QLineEdit* m_startAngleEdit;
    QLineEdit* m_endRadiusEdit;
    QLineEdit* m_endAngleEdit;
};
