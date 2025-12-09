#pragma once
#include "BasePropertiesWidget.h"
#include "PointPrimitive.h"

#include <QLineEdit>

class RectanglePrimitive;

class RectanglePropertiesWidget : public BasePropertiesWidget
{
    Q_OBJECT
public:
    explicit RectanglePropertiesWidget(QWidget* parent = nullptr);
    void setPrimitives(const QList<BasePrimitive*>& primitives) override;

signals:
    void propertiesApplied(RectanglePrimitive* rect, const PointPrimitive& center, double w, double h, double rotation, const QColor& color, LineType type);

private slots:
    void onApplyButtonClicked();

private:
    void updateFieldValues() override;

    RectanglePrimitive* m_currentRect = nullptr;

    QLineEdit* m_centerX;
    QLineEdit* m_centerY;
    QLineEdit* m_width;
    QLineEdit* m_height;
    QLineEdit* m_rotation;
};
