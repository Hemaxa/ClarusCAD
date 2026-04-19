// DimensionPropertiesWidget.cpp

#include "DimensionPropertiesWidget.h"
#include "BaseDimensionPrimitive.h"
#include "LineStyleManager.h"
#include "SettingsManager.h"

#include <QCheckBox>
#include <QColorDialog>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QFontComboBox>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QSignalBlocker>
#include <QStackedWidget>
#include <QVBoxLayout>

namespace {
QWidget* createColumn(const QString& title, QHBoxLayout* parentLayout)
{
    auto* column = new QWidget();
    column->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum);
    column->setMinimumWidth(166);

    auto* columnLayout = new QVBoxLayout(column);
    columnLayout->setContentsMargins(0, 0, 0, 0);
    columnLayout->setSpacing(2);

    auto* titleLabel = new QLabel(title);
    titleLabel->setStyleSheet("font-weight: 600;");
    columnLayout->addWidget(titleLabel);

    auto* formLayout = new QFormLayout();
    formLayout->setContentsMargins(0, 0, 0, 0);
    formLayout->setHorizontalSpacing(6);
    formLayout->setVerticalSpacing(2);
    formLayout->setLabelAlignment(Qt::AlignRight | Qt::AlignVCenter);
    formLayout->setFieldGrowthPolicy(QFormLayout::FieldsStayAtSizeHint);
    columnLayout->addLayout(formLayout);

    parentLayout->addWidget(column);
    return column;
}

QFormLayout* formOf(QWidget* column)
{
    auto* columnLayout = qobject_cast<QVBoxLayout*>(column->layout());
    return static_cast<QFormLayout*>(columnLayout->itemAt(1)->layout());
}
}

DimensionPropertiesWidget::DimensionPropertiesWidget(QWidget* parent) : BasePropertiesWidget(parent)
{
    m_paramsStack->hide();
    m_rightColumn->hide();

    auto* centralLayout = qobject_cast<QVBoxLayout*>(m_centralColumn->layout());
    auto* columnsLayout = new QHBoxLayout();
    columnsLayout->setContentsMargins(0, 0, 0, 0);
    columnsLayout->setSpacing(8);

    auto* valueGroup = createColumn("Значение", columnsLayout);
    auto* textGroup = createColumn("Текст", columnsLayout);
    auto* extensionGroup = createColumn("Выносные", columnsLayout);
    auto* dimensionLineGroup = createColumn("Размерная", columnsLayout);
    auto* arrowGroup = createColumn("Стрелки", columnsLayout);

    m_typeLabel = new QLabel("-", this);
    m_measuredValueLabel = new QLabel("0.00", this);
    m_measuredValueSpinBox = new QDoubleSpinBox(this);
    m_measuredValueSpinBox->setRange(0.001, 1000000.0);
    m_measuredValueSpinBox->setDecimals(3);
    m_measuredValueSpinBox->setSingleStep(1.0);
    m_measuredValueSpinBox->setToolTip("Меняет измеряемую геометрию выбранного размера.");
    m_measuredValueSpinBox->setObjectName("PropertiesInput");

    m_customTextEdit = new QLineEdit(this);
    m_customTextEdit->setPlaceholderText("Авто: вычисленное значение");
    m_customTextEdit->setMinimumWidth(150);
    m_customTextEdit->setMaximumWidth(220);

    m_layerEdit = new QLineEdit("0", this);
    m_layerEdit->setPlaceholderText("0");
    m_layerEdit->setMaximumWidth(90);

    m_fontComboBox = new QFontComboBox(this);
    m_fontComboBox->setMinimumWidth(150);
    m_fontComboBox->setMaximumWidth(220);

    m_textHeightSpinBox = new QDoubleSpinBox(this);
    m_textHeightSpinBox->setRange(6.0, 48.0);
    m_textHeightSpinBox->setSuffix(" px");
    m_textHeightSpinBox->setObjectName("PropertiesInput");

    m_textGapSpinBox = new QDoubleSpinBox(this);
    m_textGapSpinBox->setRange(0.0, 40.0);
    m_textGapSpinBox->setSuffix(" px");
    m_textGapSpinBox->setObjectName("PropertiesInput");

    m_textAlongOffsetSpinBox = new QDoubleSpinBox(this);
    m_textAlongOffsetSpinBox->setRange(-100.0, 100.0);
    m_textAlongOffsetSpinBox->setSuffix(" px");
    m_textAlongOffsetSpinBox->setObjectName("PropertiesInput");

    m_arrowSizeSpinBox = new QDoubleSpinBox(this);
    m_arrowSizeSpinBox->setRange(4.0, 40.0);
    m_arrowSizeSpinBox->setSuffix(" px");
    m_arrowSizeSpinBox->setObjectName("PropertiesInput");

    m_arrowTypeComboBox = new QComboBox(this);
    m_arrowTypeComboBox->setObjectName("PropertiesComboBox");
    m_arrowTypeComboBox->addItem("Классическая (закрытая)", static_cast<int>(DimensionArrowType::ClosedFilled));
    m_arrowTypeComboBox->addItem("Разомкнутая (открытая)", static_cast<int>(DimensionArrowType::ClosedOpen));
    m_arrowTypeComboBox->addItem("Засечка", static_cast<int>(DimensionArrowType::Slash));
    m_arrowTypeComboBox->addItem("Точка", static_cast<int>(DimensionArrowType::Dot));

    m_arrowFilledCheckBox = new QCheckBox("Заполнение стрелок", this);

    m_extensionOffsetSpinBox = new QDoubleSpinBox(this);
    m_extensionOffsetSpinBox->setRange(0.0, 100.0);
    m_extensionOffsetSpinBox->setSuffix(" px");
    m_extensionOffsetSpinBox->setObjectName("PropertiesInput");

    m_extensionExtendSpinBox = new QDoubleSpinBox(this);
    m_extensionExtendSpinBox->setRange(0.0, 100.0);
    m_extensionExtendSpinBox->setSuffix(" px");
    m_extensionExtendSpinBox->setObjectName("PropertiesInput");

    m_dimensionLineExtendSpinBox = new QDoubleSpinBox(this);
    m_dimensionLineExtendSpinBox->setRange(0.0, 100.0);
    m_dimensionLineExtendSpinBox->setSuffix(" px");
    m_dimensionLineExtendSpinBox->setObjectName("PropertiesInput");

    m_extensionLineTypeCombo = new QComboBox(this);
    m_dimensionLineTypeCombo = new QComboBox(this);
    m_extensionLineTypeCombo->setObjectName("PropertiesComboBox");
    m_dimensionLineTypeCombo->setObjectName("PropertiesComboBox");
    populateLocalLineTypeCombo(m_extensionLineTypeCombo);
    populateLocalLineTypeCombo(m_dimensionLineTypeCombo);

    m_textColorButton = new QPushButton(this);
    m_extensionColorButton = new QPushButton(this);
    m_dimensionLineColorButton = new QPushButton(this);
    for (auto* button : {m_textColorButton, m_extensionColorButton, m_dimensionLineColorButton}) {
        button->setFixedHeight(24);
        button->setMaximumWidth(96);
    }

    formOf(valueGroup)->addRow("Тип:", m_typeLabel);
    formOf(valueGroup)->addRow("Авто:", m_measuredValueLabel);
    formOf(valueGroup)->addRow("Задать:", m_measuredValueSpinBox);
    formOf(valueGroup)->addRow("Слой:", m_layerEdit);

    formOf(textGroup)->addRow("Текст:", m_customTextEdit);
    formOf(textGroup)->addRow("Шрифт:", m_fontComboBox);
    formOf(textGroup)->addRow("Высота:", m_textHeightSpinBox);
    formOf(textGroup)->addRow("Отступ:", m_textGapSpinBox);
    formOf(textGroup)->addRow("Вдоль:", m_textAlongOffsetSpinBox);
    formOf(textGroup)->addRow("Цвет:", m_textColorButton);

    formOf(extensionGroup)->addRow("Цвет:", m_extensionColorButton);
    formOf(extensionGroup)->addRow("Тип линии:", m_extensionLineTypeCombo);
    formOf(extensionGroup)->addRow("Отступ:", m_extensionOffsetSpinBox);
    formOf(extensionGroup)->addRow("Выход:", m_extensionExtendSpinBox);

    formOf(dimensionLineGroup)->addRow("Цвет:", m_dimensionLineColorButton);
    formOf(dimensionLineGroup)->addRow("Тип линии:", m_dimensionLineTypeCombo);
    formOf(dimensionLineGroup)->addRow("Расширение:", m_dimensionLineExtendSpinBox);

    formOf(arrowGroup)->addRow("Тип:", m_arrowTypeComboBox);
    formOf(arrowGroup)->addRow("Размер:", m_arrowSizeSpinBox);
    formOf(arrowGroup)->addRow("", m_arrowFilledCheckBox);

    columnsLayout->addStretch();
    centralLayout->addLayout(columnsLayout);

    connect(m_customTextEdit, &QLineEdit::textEdited, this, &DimensionPropertiesWidget::onCustomTextChanged);
    connect(m_measuredValueSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &DimensionPropertiesWidget::onMeasuredValueChanged);
    connect(m_layerEdit, &QLineEdit::editingFinished, this, &DimensionPropertiesWidget::onLayerEdited);
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
    if (!m_currentPrimitive) {
        const DimensionStyle style = SettingsManager::instance().getDefaultDimensionStyle();
        QSignalBlocker b1(m_customTextEdit), b2(m_fontComboBox), b3(m_textHeightSpinBox), b4(m_textGapSpinBox),
            b5(m_textAlongOffsetSpinBox), b6(m_arrowSizeSpinBox), b7(m_arrowTypeComboBox), b8(m_arrowFilledCheckBox),
            b9(m_extensionOffsetSpinBox), b10(m_extensionExtendSpinBox), b11(m_dimensionLineExtendSpinBox),
            b12(m_extensionLineTypeCombo), b13(m_dimensionLineTypeCombo), b14(m_measuredValueSpinBox), b15(m_layerEdit);
        m_typeLabel->setText("-");
        m_measuredValueLabel->setText("Создание нового размера");
        m_measuredValueSpinBox->setValue(1.0);
        m_customTextEdit->clear();
        m_layerEdit->setText("0");
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
        return;
    }

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
        b12(m_extensionLineTypeCombo), b13(m_dimensionLineTypeCombo), b14(m_measuredValueSpinBox), b15(m_layerEdit);

    m_customTextEdit->setText(dim->getCustomText());
    m_measuredValueSpinBox->setValue(dim->getMeasuredValue());
    m_layerEdit->setText(m_currentPrimitive->getLayerName());
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

void DimensionPropertiesWidget::onMeasuredValueChanged(double value)
{
    if (m_selectedPrimitives.isEmpty()) return;

    for (auto* prim : m_selectedPrimitives) {
        const PrimitiveType type = prim->getType();
        if (type == PrimitiveType::LinearDimension
            || type == PrimitiveType::RadialDimension
            || type == PrimitiveType::AngularDimension) {
            auto* dim = static_cast<BaseDimensionPrimitive*>(prim);
            if (dim->applyMeasuredValueOverride(value)) {
                dim->setCustomText(QString());
            }
        }
    }
    updateFieldValues();
    emit dimensionPropertiesApplied();
}

void DimensionPropertiesWidget::onLayerEdited()
{
    QString layerName = m_layerEdit->text().trimmed();
    if (layerName.isEmpty()) {
        layerName = "0";
        m_layerEdit->setText(layerName);
    }

    for (auto* prim : m_selectedPrimitives) {
        if (prim) prim->setLayerName(layerName);
    }
    emit layerChanged(layerName);
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
