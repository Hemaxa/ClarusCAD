// DimensionPropertiesWidget.h

#pragma once

#include "BasePropertiesWidget.h"
#include <QLabel>
#include <QLineEdit>

class LinearDimensionPrimitive;

class DimensionPropertiesWidget : public BasePropertiesWidget
{
    Q_OBJECT

public:
    explicit DimensionPropertiesWidget(QWidget* parent = nullptr);
    virtual ~DimensionPropertiesWidget() = default;

signals:
    void dimensionPropertiesApplied();


protected:
    virtual void updateFieldValues() override;

private slots:
    void onCustomTextChanged(const QString& text);

private:
    QLabel* m_measuredValueLabel;
    QLineEdit* m_customTextEdit;
};
