// DimensionPropertiesWidget.cpp

#include "DimensionPropertiesWidget.h"
#include "BaseDimensionPrimitive.h"
#include "../../model/primitives/dimensions/AngularDimensionPrimitive.h"
#include "../../model/primitives/dimensions/LinearDimensionPrimitive.h"
#include "LineStyleManager.h"
#include "SettingsManager.h"
#include "ThemeManager.h"

#include <QColorDialog>
#include <QCheckBox>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSignalBlocker>
#include <QStackedWidget>
#include <QVBoxLayout>
#include <algorithm>

namespace {
QWidget* createColumn(const QString& title, QHBoxLayout* parentLayout, int minWidth = 180)
{
    auto* column = new QWidget();
    column->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum);
    column->setMinimumWidth(minWidth);

    auto* columnLayout = new QVBoxLayout(column);
    columnLayout->setContentsMargins(0, 0, 0, 0);
    columnLayout->setSpacing(4);

    auto* titleLabel = new QLabel(title);
    titleLabel->setStyleSheet("font-weight: 600;");
    columnLayout->addWidget(titleLabel);

    auto* formLayout = new QFormLayout();
    formLayout->setContentsMargins(0, 0, 0, 0);
    formLayout->setHorizontalSpacing(8);
    formLayout->setVerticalSpacing(4);
    formLayout->setLabelAlignment(Qt::AlignRight | Qt::AlignVCenter);
    formLayout->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);
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
    columnsLayout->setSpacing(12);

    auto* valueGroup = createColumn("Значение", columnsLayout, 220);
    auto* styleGroup = createColumn("Линии и стрелки", columnsLayout, 250);
    auto* colorGroup = createColumn("Цвета", columnsLayout, 220);

    m_typeLabel = new QLabel("-", this);
    m_measuredValueLabel = new QLabel("0.00", this);

    m_measuredValueSpinBox = new QDoubleSpinBox(this);
    m_measuredValueSpinBox->setRange(0.001, 1000000.0);
    m_measuredValueSpinBox->setDecimals(3);
    m_measuredValueSpinBox->setSingleStep(1.0);
    m_measuredValueSpinBox->setObjectName("PropertiesInput");
    m_measuredValueSpinBox->setToolTip("Меняет измеряемую геометрию выбранного размера.");

    m_prefixEdit = new QLineEdit(this);
    m_prefixEdit->setPlaceholderText("Например: R или Ø");

    m_customTextEdit = new QLineEdit(this);
    m_customTextEdit->setPlaceholderText("Авто: вычисленное значение");

    m_layerEdit = new QLineEdit("0", this);
    m_layerEdit->setPlaceholderText("0");

    m_extensionLineTypeCombo = new QComboBox(this);
    m_dimensionLineTypeCombo = new QComboBox(this);
    m_arrowTypeComboBox = new QComboBox(this);
    m_shelfCheckBox = new QCheckBox("Добавить полку", this);
    for (auto* combo : {m_extensionLineTypeCombo, m_dimensionLineTypeCombo, m_arrowTypeComboBox}) {
        combo->setObjectName("PropertiesComboBox");
        combo->setMinimumWidth(170);
    }

    populateLocalLineTypeCombo(m_extensionLineTypeCombo);
    populateLocalLineTypeCombo(m_dimensionLineTypeCombo);
    populateArrowTypeCombo();

    m_textColorButton = new QPushButton(this);
    m_extensionColorButton = new QPushButton(this);
    m_dimensionLineColorButton = new QPushButton(this);
    m_toggleAngularArcSideButton = new QPushButton("Сменить сторону", this);
    for (auto* button : {m_textColorButton, m_extensionColorButton, m_dimensionLineColorButton}) {
        button->setFixedHeight(24);
        button->setMinimumWidth(120);
    }

    formOf(valueGroup)->addRow("Тип:", m_typeLabel);
    formOf(valueGroup)->addRow("Авто:", m_measuredValueLabel);
    formOf(valueGroup)->addRow("Задать:", m_measuredValueSpinBox);
    formOf(valueGroup)->addRow("Префикс:", m_prefixEdit);
    formOf(valueGroup)->addRow("Текст:", m_customTextEdit);
    formOf(valueGroup)->addRow("Слой:", m_layerEdit);
    formOf(valueGroup)->addRow("", m_toggleAngularArcSideButton);

    formOf(styleGroup)->addRow("Выносные:", m_extensionLineTypeCombo);
    formOf(styleGroup)->addRow("Размерная:", m_dimensionLineTypeCombo);
    formOf(styleGroup)->addRow("Стрелки:", m_arrowTypeComboBox);
    formOf(styleGroup)->addRow("", m_shelfCheckBox);

    formOf(colorGroup)->addRow("Текст:", m_textColorButton);
    formOf(colorGroup)->addRow("Выносные:", m_extensionColorButton);
    formOf(colorGroup)->addRow("Размерная:", m_dimensionLineColorButton);

    columnsLayout->addStretch();
    centralLayout->addLayout(columnsLayout);

    connect(m_prefixEdit, &QLineEdit::textEdited, this, &DimensionPropertiesWidget::onPrefixChanged);
    connect(m_customTextEdit, &QLineEdit::textEdited, this, &DimensionPropertiesWidget::onCustomTextChanged);
    connect(m_measuredValueSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &DimensionPropertiesWidget::onMeasuredValueChanged);
    connect(m_layerEdit, &QLineEdit::editingFinished, this, &DimensionPropertiesWidget::onLayerEdited);
    connect(m_arrowTypeComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this]() { onStyleValueChanged(); });
    connect(m_extensionLineTypeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this]() { onStyleValueChanged(); });
    connect(m_dimensionLineTypeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this]() { onStyleValueChanged(); });
    connect(m_textColorButton, &QPushButton::clicked, this, &DimensionPropertiesWidget::onTextColorClicked);
    connect(m_extensionColorButton, &QPushButton::clicked, this, &DimensionPropertiesWidget::onExtensionColorClicked);
    connect(m_dimensionLineColorButton, &QPushButton::clicked, this, &DimensionPropertiesWidget::onDimensionLineColorClicked);
    connect(m_toggleAngularArcSideButton, &QPushButton::clicked, this, &DimensionPropertiesWidget::onToggleAngularArcSideClicked);
    connect(m_shelfCheckBox, &QCheckBox::toggled, this, [this]() { onStyleValueChanged(); });
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

void DimensionPropertiesWidget::populateArrowTypeCombo()
{
    const QVariant currentValue = m_arrowTypeComboBox->currentData();
    m_arrowTypeComboBox->blockSignals(true);
    m_arrowTypeComboBox->clear();

    const QColor iconColor = ThemeManager::instance().getIconColor();
    struct ArrowOption {
        DimensionArrowType type;
        const char* title;
        const char* iconPath;
    };

    const ArrowOption options[] = {
        {DimensionArrowType::ClosedFilled, "Закрытая", ":/icons/icons/arrows/arrow-open.svg"},
        {DimensionArrowType::ClosedOpen, "Открытая", ":/icons/icons/arrows/arrow-closed.svg"},
        {DimensionArrowType::Slash, "Засечка", ":/icons/icons/arrows/arrow-tick.svg"},
        {DimensionArrowType::Dot, "Точка", ":/icons/icons/arrows/arrow-dot.svg"},
    };

    for (const auto& option : options) {
        const QIcon icon = ThemeManager::colorizeSvg(option.iconPath, iconColor);
        m_arrowTypeComboBox->addItem(icon, QString::fromUtf8(option.title), static_cast<int>(option.type));
    }

    int currentIndex = m_arrowTypeComboBox->findData(currentValue);
    if (currentIndex < 0) {
        currentIndex = m_arrowTypeComboBox->findData(static_cast<int>(DimensionArrowType::ClosedFilled));
    }
    m_arrowTypeComboBox->setCurrentIndex(std::max(0, currentIndex));
    m_arrowTypeComboBox->blockSignals(false);
}

void DimensionPropertiesWidget::updateColorButton(QPushButton* button, const QColor& color)
{
    button->setProperty("selectedColor", color);
    button->setStyleSheet(QString("background-color: %1;").arg(color.name()));
    button->setText(color.name());
}

void DimensionPropertiesWidget::updateFieldValues()
{
    const DimensionStyle style = m_currentPrimitive
        ? static_cast<BaseDimensionPrimitive*>(m_currentPrimitive)->getStyle()
        : SettingsManager::instance().getDefaultDimensionStyle();

    QSignalBlocker b1(m_customTextEdit);
    QSignalBlocker b2(m_prefixEdit);
    QSignalBlocker b3(m_measuredValueSpinBox);
    QSignalBlocker b4(m_layerEdit);
    QSignalBlocker b5(m_arrowTypeComboBox);
    QSignalBlocker b6(m_extensionLineTypeCombo);
    QSignalBlocker b7(m_dimensionLineTypeCombo);
    QSignalBlocker b8(m_shelfCheckBox);

    if (!m_currentPrimitive) {
        m_typeLabel->setText("-");
        m_measuredValueLabel->setText("Создание нового размера");
        m_measuredValueSpinBox->setValue(1.0);
        m_prefixEdit->clear();
        m_customTextEdit->clear();
        m_layerEdit->setText("0");
    } else {
        const PrimitiveType type = m_currentPrimitive->getType();
        if (type != PrimitiveType::LinearDimension
            && type != PrimitiveType::RadialDimension
            && type != PrimitiveType::AngularDimension) {
            return;
        }

        auto* dim = static_cast<BaseDimensionPrimitive*>(m_currentPrimitive);
        m_typeLabel->setText(m_currentPrimitive->getTypeName());
        m_measuredValueLabel->setText(QString::number(dim->getMeasuredValue(), 'f', 2));
        m_measuredValueSpinBox->setValue(dim->getMeasuredValue());
        if (type == PrimitiveType::LinearDimension) {
            m_prefixEdit->setText(static_cast<LinearDimensionPrimitive*>(m_currentPrimitive)->getTextPrefix());
        } else {
            m_prefixEdit->clear();
        }
        m_customTextEdit->setText(dim->getCustomText());
        m_layerEdit->setText(m_currentPrimitive->getLayerName());
    }

    m_toggleAngularArcSideButton->setVisible(m_currentPrimitive && m_currentPrimitive->getType() == PrimitiveType::AngularDimension);
    m_prefixEdit->setEnabled(m_currentPrimitive && m_currentPrimitive->getType() == PrimitiveType::LinearDimension);
    m_arrowTypeComboBox->setCurrentIndex(m_arrowTypeComboBox->findData(static_cast<int>(style.arrowType)));
    m_extensionLineTypeCombo->setCurrentIndex(m_extensionLineTypeCombo->findData(style.extensionLineTypeId));
    m_dimensionLineTypeCombo->setCurrentIndex(m_dimensionLineTypeCombo->findData(style.dimensionLineTypeId));
    m_shelfCheckBox->setChecked(m_currentPrimitive && static_cast<BaseDimensionPrimitive*>(m_currentPrimitive)->hasShelf());
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

void DimensionPropertiesWidget::onPrefixChanged(const QString& text)
{
    if (m_selectedPrimitives.isEmpty()) return;

    for (auto* prim : m_selectedPrimitives) {
        if (prim && prim->getType() == PrimitiveType::LinearDimension) {
            static_cast<LinearDimensionPrimitive*>(prim)->setTextPrefix(text);
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
        style.arrowType = static_cast<DimensionArrowType>(m_arrowTypeComboBox->currentData().toInt());
        style.extensionLineTypeId = m_extensionLineTypeCombo->currentData().toInt();
        style.dimensionLineTypeId = m_dimensionLineTypeCombo->currentData().toInt();
        style.textColor = m_textColorButton->property("selectedColor").value<QColor>();
        style.extensionLineColor = m_extensionColorButton->property("selectedColor").value<QColor>();
        style.dimensionLineColor = m_dimensionLineColorButton->property("selectedColor").value<QColor>();
        dim->setStyle(style);
        dim->setShelfEnabled(m_shelfCheckBox->isChecked());
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

void DimensionPropertiesWidget::onToggleAngularArcSideClicked()
{
    if (m_selectedPrimitives.isEmpty()) return;

    for (auto* prim : m_selectedPrimitives) {
        if (prim && prim->getType() == PrimitiveType::AngularDimension) {
            static_cast<AngularDimensionPrimitive*>(prim)->toggleArcSide();
        }
    }

    updateFieldValues();
    emit angularDimensionArcSideToggled();
    emit dimensionPropertiesApplied();
}

void DimensionPropertiesWidget::updateColors()
{
    BasePropertiesWidget::updateColors();
    populateArrowTypeCombo();
}
