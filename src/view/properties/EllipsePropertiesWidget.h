//EllipsePropertiesWidget - виджет свойств примитива "Эллипс"

#pragma once
#include "BasePropertiesWidget.h"
#include "PointPrimitive.h"
#include <QLineEdit>

class EllipsePrimitive;

class EllipsePropertiesWidget : public BasePropertiesWidget
{
    Q_OBJECT
public:
    /**
     * @brief Конструктор виджета свойств эллипса.
     */
    explicit EllipsePropertiesWidget(QWidget* parent = nullptr);

    /**
     * @brief Установить редактируемые примитивы.
     */
    void setPrimitives(const QList<BasePrimitive*>& primitives) override;

signals:
    /**
     * @brief Сигнал применения данных (для создания или обновления).
     */
    void propertiesApplied(EllipsePrimitive* ell, const PointPrimitive& center, double rx, double ry, double rot, const QColor& c, LineType t);

private slots:
    void onApplyButtonClicked();

private:
    void updateFieldValues() override;

    EllipsePrimitive* m_currentEllipse = nullptr; ///< Текущий редактируемый эллипс

    QLineEdit* m_centerX;
    QLineEdit* m_centerY;
    QLineEdit* m_radiusX;
    QLineEdit* m_radiusY;
    QLineEdit* m_rotation;
};
