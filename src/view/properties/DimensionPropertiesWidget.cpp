// DimensionPropertiesWidget.cpp

#include "DimensionPropertiesWidget.h"
#include "BaseDimensionPrimitive.h"
#include "LineStyleManager.h"

#include <QCheckBox>
#include <QColorDialog>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QFontComboBox>
#include <QFormLayout>
#include <QPushButton>
#include <QSignalBlocker>

DimensionPropertiesWidget::DimensionPropertiesWidget(QWidget* parent) : BasePropertiesWidget(parent)
{
    auto* layout = new QFormLayout(m_centralColumn);

    m_typeLabel = new QLabel("-", this);
    m_measuredValueLabel = new QLabel("0.00", this);
    m_customTextEdit = new QLineEdit(this);
    m_customTextEdit->setPlaceholderText("Авто (поставьте текст для переопределения)");

    m_fontComboBox = new QFontComboBox(this);

    m_textHeightSpinBox = new QDoubleSpinBox(this);
    m_textHeightSpinBox->setRange(6.0, 48.0);
    m_textHeightSpinBox->setSuffix(" px");

    m_textGapSpinBox = new QDoubleSpinBox(this);
    m_textGapSpinBox->setRange(0.0, 40.0);
    m_textGapSpinBox->setSuffix(" px");

    m_textAlongOffsetSpinBox = new QDoubleSpinBox(this);
    m_textAlongOffsetSpinBox->setRange(-100.0, 100.0);
    m_textAlongOffsetSpinBox->setSuffix(" px");

    m_arrowSizeSpinBox = new QDoubleSpinBox(this);
    m_arrowSizeSpinBox->setRange(4.0, 40.0);
    m_arrowSizeSpinBox->setSuffix(" px");

    m_arrowTypeComboBox = new QComboBox(this);
    m_arrowTypeComboBox->addItem("Закрытая", static_cast<int>(DimensionArrowType::ClosedFilled));
    m_arrowTypeComboBox->addItem("Открытая", static_cast<int>(DimensionArrowType::ClosedOpen));
    m_arrowTypeComboBox->addItem("Засечка", static_cast<int>(DimensionArrowType::Slash));

    m_arrowFilledCheckBox = new QCheckBox("Заполнение стрелок", this);

    m_extensionOffsetSpinBox = new QDoubleSpinBox(this);
    m_extensionOffsetSpinBox->setRange(0.0, 100.0);
    m_extensionOffsetSpinBox->setSuffix(" px");

    m_extensionExtendSpinBox = new QDoubleSpinBox(this);
    m_extensionExtendSpinBox->setRange(0.0, 100.0);
    m_extensionExtendSpinBox->setSuffix(" px");

    m_dimensionLineExtendSpinBox = new QDoubleSpinBox(this);
    m_dimensionLineExtendSpinBox->setRange(0.0, 100.0);
    m_dimensionLineExtendSpinBox->setSuffix(" px");

    m_extensionLineTypeCombo = new QComboBox(this);
    m_dimensionLineTypeCombo = new QComboBox(this);
    populateLocalLineTypeCombo(m_extensionLineTypeCombo);
    populateLocalLineTypeCombo(m_dimensionLineTypeCombo);

    m_textColorButton = new QPushButton(this);
    m_extensionColorButton = new QPushButton(this);
    m_dimensionLineColorButton = new QPushButton(this);

    layout->addRow("Тип размера:", m_typeLabel);
    layout->addRow("Измеренное значение:", m_measuredValueLabel);
    layout->addRow("Текст размера:", m_customTextEdit);
    layout->addRow("Шрифт:", m_fontComboBox);
    layout->addRow("Высота текста:", m_textHeightSpinBox);
    layout->addRow("Отступ текста:", m_textGapSpinBox);
    layout->addRow("Смещение вдоль линии:", m_textAlongOffsetSpinBox);
    layout->addRow("Размер стрелки:", m_arrowSizeSpinBox);
    layout->addRow("Тип стрелки:", m_arrowTypeComboBox);
    layout->addRow("", m_arrowFilledCheckBox);
    layout->addRow("Отступ выносной:", m_extensionOffsetSpinBox);
    layout->addRow("Выход выносной:", m_extensionExtendSpinBox);
    layout->addRow("Выход размерной:", m_dimensionLineExtendSpinBox);
    layout->addRow("Тип выносной:", m_extensionLineTypeCombo);
    layout->addRow("Тип размерной:", m_dimensionLineTypeCombo);
    layout->addRow("Цвет текста:", m_textColorButton);
    layout->addRow("Цвет выносной:", m_extensionColorButton);
    layout->addRow("Цвет размерной:", m_dimensionLineColorButton);

    connect(m_customTextEdit, &QLineEdit::textEdited, this, &DimensionPropertiesWidget::onCustomTextChanged);
    connect(m_fontComboBox, &QFontComboBox::currentFontChanged, this, [this]() { onStyleValueChanged(); });
    connect(m_textHeightSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [this]() { onStyleValueChanged(); });
    connect(m_textGapSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [this]() { onStyleValueChanged(); });
    connect(m_textAlongOffsetSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [this]() { onStyleValueChanged(); });
    connect(m_arrowSizeSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [this]() { onStyleValueChanged(); });
    connect(m_arrowTypeComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this]() { onStyleValueChanged(); });
    connect(m_arrowFilledCheckBox, &QCheckBox::toggled, this, [this]() { onStyleValueChanged(); });
    connect(m_extensionOffsetSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [this]() { onStyleValueChanged(); });
    connect(m_extensionExtendSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [this]() { onStyleValueChanged(); });
    connect(m_dimensionLineExtendSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [this]() { onStyleValueChanged(); });
    connect(m_extensionLineTypeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this]() { onStyleValueChanged(); });
    connect(m_dimensionLineTypeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this]() { onStyleValueChanged(); });
    connect(m_textColorButton, &QPushButton::clicked, this, &DimensionPropertiesWidget::onTextColorClicked);
    connect(m_extensionColorButton, &QPushButton::clicked, this, &DimensionPropertiesWidget::onExtensionColorClicked);
    connect(m_dimensionLineColorButton, &QPushButton::clicked, this, &DimensionPropertiesWidget::onDimensionLineColorClicked);
}

void DimensionPropertiesWidget::populateLocalLineTypeCombo(QComboBox* combo)
{
    combo->clear();
    combo->addItem("Сплошная", static_cast<int>(LineType::SolidMain));
    combo->addItem("Тонкая", static_cast<int>(LineType::SolidThin));
    combo->addItem("Штриховая", static_cast<int>(LineType::Dashed));
    combo->addItem("Штрих-пунктирная", static_cast<int>(LineType::DashDotThin));
    combo->addItem("Две точки", static_cast<int>(LineType::DashDotDot));

    auto customStyles = LineStyleManager::instance().getCustomStyles();
    for (auto it = customStyles.begin(); it != customStyles.end(); ++it) {
        combo->addItem(it.value().name, it.key());
    }
}

void DimensionPropertiesWidget::updateColorButton(QPushButton* button, const QColor& color)
{
    button->setProperty("selectedColor", color);
    button->setStyleSheet(QString("background-color: %1;").arg(color.name()));
    button->setText(color.name());
}

void DimensionPropertiesWidget::updateFieldValues()
{
    if (!m_currentPrimitive) return;

    const PrimitiveType type = m_currentPrimitive->getType();
    if (type != PrimitiveType::LinearDimension
        && type != PrimitiveType::RadialDimension
        && type != PrimitiveType::AngularDimension) {
        return;
    }

    auto* dim = static_cast<BaseDimensionPrimitive*>(m_currentPrimitive);
    const DimensionStyle style = dim->getStyle();

    m_typeLabel->setText(m_currentPrimitive->getTypeName());
    m_measuredValueLabel->setText(QString::number(dim->getMeasuredValue(), 'f', 2));

    QSignalBlocker b1(m_customTextEdit), b2(m_fontComboBox), b3(m_textHeightSpinBox), b4(m_textGapSpinBox),
        b5(m_textAlongOffsetSpinBox), b6(m_arrowSizeSpinBox), b7(m_arrowTypeComboBox), b8(m_arrowFilledCheckBox),
        b9(m_extensionOffsetSpinBox), b10(m_extensionExtendSpinBox), b11(m_dimensionLineExtendSpinBox),
        b12(m_extensionLineTypeCombo), b13(m_dimensionLineTypeCombo);

    m_customTextEdit->setText(dim->getCustomText());
    m_fontComboBox->setCurrentFont(QFont(style.fontFamily));
    m_textHeightSpinBox->setValue(style.textHeight);
    m_textGapSpinBox->setValue(style.textGap);
    m_textAlongOffsetSpinBox->setValue(style.textAlongLineOffset);
    m_arrowSizeSpinBox->setValue(style.arrowSize);
    m_arrowTypeComboBox->setCurrentIndex(m_arrowTypeComboBox->findData(static_cast<int>(style.arrowType)));
    m_arrowFilledCheckBox->setChecked(style.arrowFilled);
    m_extensionOffsetSpinBox->setValue(style.extensionLineOffset);
    m_extensionExtendSpinBox->setValue(style.extensionLineExtend);
    m_dimensionLineExtendSpinBox->setValue(style.dimensionLineExtension);
    m_extensionLineTypeCombo->setCurrentIndex(m_extensionLineTypeCombo->findData(style.extensionLineTypeId));
    m_dimensionLineTypeCombo->setCurrentIndex(m_dimensionLineTypeCombo->findData(style.dimensionLineTypeId));

    updateColorButton(m_textColorButton, style.textColor);
    updateColorButton(m_extensionColorButton, style.extensionLineColor);
    updateColorButton(m_dimensionLineColorButton, style.dimensionLineColor);
}

void DimensionPropertiesWidget::onCustomTextChanged(const QString& text)
{
    if (m_selectedPrimitives.isEmpty()) return;

    for (auto* prim : m_selectedPrimitives) {
        const PrimitiveType type = prim->getType();
        if (type == PrimitiveType::LinearDimension
            || type == PrimitiveType::RadialDimension
            || type == PrimitiveType::AngularDimension) {
            static_cast<BaseDimensionPrimitive*>(prim)->setCustomText(text);
        }
    }
    emit dimensionPropertiesApplied();
}

void DimensionPropertiesWidget::applyStyleToSelection()
{
    if (m_selectedPrimitives.isEmpty()) return;

    for (auto* prim : m_selectedPrimitives) {
        const PrimitiveType type = prim->getType();
        if (type != PrimitiveType::LinearDimension
            && type != PrimitiveType::RadialDimension
            && type != PrimitiveType::AngularDimension) {
            continue;
        }

        auto* dim = static_cast<BaseDimensionPrimitive*>(prim);
        DimensionStyle style = dim->getStyle();
        style.fontFamily = m_fontComboBox->currentFont().family();
        style.textHeight = m_textHeightSpinBox->value();
        style.textGap = m_textGapSpinBox->value();
        style.textAlongLineOffset = m_textAlongOffsetSpinBox->value();
        style.arrowSize = m_arrowSizeSpinBox->value();
        style.arrowType = static_cast<DimensionArrowType>(m_arrowTypeComboBox->currentData().toInt());
        style.arrowFilled = m_arrowFilledCheckBox->isChecked();
        style.extensionLineOffset = m_extensionOffsetSpinBox->value();
        style.extensionLineExtend = m_extensionExtendSpinBox->value();
        style.dimensionLineExtension = m_dimensionLineExtendSpinBox->value();
        style.extensionLineTypeId = m_extensionLineTypeCombo->currentData().toInt();
        style.dimensionLineTypeId = m_dimensionLineTypeCombo->currentData().toInt();
        style.textColor = m_textColorButton->property("selectedColor").value<QColor>();
        style.extensionLineColor = m_extensionColorButton->property("selectedColor").value<QColor>();
        style.dimensionLineColor = m_dimensionLineColorButton->property("selectedColor").value<QColor>();
        dim->setStyle(style);
    }
}

void DimensionPropertiesWidget::onStyleValueChanged()
{
    applyStyleToSelection();
    emit dimensionPropertiesApplied();
}

void DimensionPropertiesWidget::onTextColorClicked()
{
    QColor color = QColorDialog::getColor(m_textColorButton->property("selectedColor").value<QColor>(), this);
    if (!color.isValid()) return;
    updateColorButton(m_textColorButton, color);
    onStyleValueChanged();
}

void DimensionPropertiesWidget::onExtensionColorClicked()
{
    QColor color = QColorDialog::getColor(m_extensionColorButton->property("selectedColor").value<QColor>(), this);
    if (!color.isValid()) return;
    updateColorButton(m_extensionColorButton, color);
    onStyleValueChanged();
}

void DimensionPropertiesWidget::onDimensionLineColorClicked()
{
    QColor color = QColorDialog::getColor(m_dimensionLineColorButton->property("selectedColor").value<QColor>(), this);
    if (!color.isValid()) return;
    updateColorButton(m_dimensionLineColorButton, color);
    onStyleValueChanged();
}
