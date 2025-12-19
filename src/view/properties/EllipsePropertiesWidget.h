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
    explicit EllipsePropertiesWidget(QWidget* parent = nullptr);
    void setPrimitives(const QList<BasePrimitive*>& primitives) override;

signals:
    void propertiesApplied(EllipsePrimitive* ell, const PointPrimitive& center, double rx, double ry, double rot, const QColor& c, LineType t);

private slots:
    void onApplyButtonClicked();

private:
    void updateFieldValues() override;

    EllipsePrimitive* m_currentEllipse = nullptr;

    QLineEdit* m_centerX;
    QLineEdit* m_centerY;
    QLineEdit* m_radiusX;
    QLineEdit* m_radiusY;
    QLineEdit* m_rotation;
};
