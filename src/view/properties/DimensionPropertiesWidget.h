// DimensionPropertiesWidget.h

#pragma once

#include "BasePropertiesWidget.h"
#include <QLabel>
#include <QLineEdit>

class QDoubleSpinBox;
class QComboBox;
class QCheckBox;
class QPushButton;
class QFontComboBox;

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

    QLabel* m_typeLabel;
    QLabel* m_measuredValueLabel;
    QDoubleSpinBox* m_measuredValueSpinBox;
    QLineEdit* m_customTextEdit;
    QLineEdit* m_layerEdit;
    QFontComboBox* m_fontComboBox;
    QDoubleSpinBox* m_textHeightSpinBox;
    QDoubleSpinBox* m_textGapSpinBox;
    QDoubleSpinBox* m_textAlongOffsetSpinBox;
    QDoubleSpinBox* m_arrowSizeSpinBox;
    QComboBox* m_arrowTypeComboBox;
    QCheckBox* m_arrowFilledCheckBox;
    QDoubleSpinBox* m_extensionOffsetSpinBox;
    QDoubleSpinBox* m_extensionExtendSpinBox;
    QDoubleSpinBox* m_dimensionLineExtendSpinBox;
    QComboBox* m_extensionLineTypeCombo;
    QComboBox* m_dimensionLineTypeCombo;
    QPushButton* m_textColorButton;
    QPushButton* m_extensionColorButton;
    QPushButton* m_dimensionLineColorButton;
};
