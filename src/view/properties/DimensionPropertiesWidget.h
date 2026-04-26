// DimensionPropertiesWidget.h

#pragma once

#include "BasePropertiesWidget.h"
#include <QLabel>
#include <QLineEdit>

class QDoubleSpinBox;
class QComboBox;
class QCheckBox;
class QPushButton;

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

public slots:
    void updateColors();

private slots:
    void onCustomTextChanged(const QString& text);
    void onMeasuredValueChanged(double value);
    void onLayerEdited();
    void onStyleValueChanged();
    void onTextColorClicked();
    void onExtensionColorClicked();
    void onDimensionLineColorClicked();

private:
    void applyStyleToSelection();
    void updateColorButton(QPushButton* button, const QColor& color);
    void populateLocalLineTypeCombo(QComboBox* combo);
    void populateArrowTypeCombo();

    QLabel* m_typeLabel;
    QLabel* m_measuredValueLabel;
    QDoubleSpinBox* m_measuredValueSpinBox;
    QLineEdit* m_customTextEdit;
    QLineEdit* m_layerEdit;
    QComboBox* m_arrowTypeComboBox;
    QComboBox* m_extensionLineTypeCombo;
    QComboBox* m_dimensionLineTypeCombo;
    QPushButton* m_textColorButton;
    QPushButton* m_extensionColorButton;
    QPushButton* m_dimensionLineColorButton;
};
