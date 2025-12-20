//ArcPropertiesWidget - виджет свойств примитива "Дуга"

#pragma once
#include "BasePropertiesWidget.h"
#include "PointPrimitive.h"

#include <QLineEdit>

class ArcPrimitive;

class ArcPropertiesWidget : public BasePropertiesWidget
{
    Q_OBJECT
public:
    /**
     * @brief Конструктор виджета свойств дуги.
     */
    explicit ArcPropertiesWidget(QWidget* parent = nullptr);

    /**
     * @brief Установить редактируемые примитивы.
     */
    void setPrimitives(const QList<BasePrimitive*>& primitives) override;

signals:
    /**
     * @brief Сигнал применения данных (для создания или обновления).
     */
    void propertiesApplied(ArcPrimitive* arc, const PointPrimitive& center, double radius, double startAngle, double spanAngle, const QColor& c, LineType t);

private slots:
    void onApplyButtonClicked();

private:
    void updateFieldValues() override;

    ArcPrimitive* m_currentArc = nullptr; ///< Текущая редактируемая дуга
    QLineEdit* m_centerX;
    QLineEdit* m_centerY;
    QLineEdit* m_radius;
    QLineEdit* m_startAngle;
    QLineEdit* m_spanAngle;
};
