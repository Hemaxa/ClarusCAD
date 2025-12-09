#pragma once
#include "BasePropertiesWidget.h"
#include "PointPrimitive.h"

#include <QLineEdit>

class ArcPrimitive;

class ArcPropertiesWidget : public BasePropertiesWidget
{
    Q_OBJECT
public:
    explicit ArcPropertiesWidget(QWidget* parent = nullptr);
    void setPrimitives(const QList<BasePrimitive*>& primitives) override;

signals:
    void propertiesApplied(ArcPrimitive* arc, const PointPrimitive& center, double radius, double startAngle, double spanAngle, const QColor& c, LineType t);

private slots:
    void onApplyButtonClicked();

private:
    void updateFieldValues() override;

    ArcPrimitive* m_currentArc = nullptr;
    QLineEdit* m_centerX;
    QLineEdit* m_centerY;
    QLineEdit* m_radius;
    QLineEdit* m_startAngle;
    QLineEdit* m_spanAngle;
};
