#include "BasePropertiesWidget.h"
#include "BasePrimitive.h"
#include "ThemeManager.h"
#include "LineStyleManager.h"

#include <QPushButton>
#include <QColorDialog>
#include <QFormLayout>
#include <QStackedWidget>
#include <QGridLayout>
#include <QLabel>
#include <QSpacerItem>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QComboBox>
#include <QLineEdit>
#include <QListView>
#include <QPainter>
#include <QSpinBox>
#include <QDoubleSpinBox>

BasePropertiesWidget::BasePropertiesWidget(QWidget* parent) : QWidget(parent)
{
    //установка системы координат, цвета и типа линии по-умолчанию
    m_selectedCoordSystem = CoordinateSystemType::Cartesian;

    //создание главной горизонтальной компоновки
    auto* mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(8, 4, 8, 4);
    mainLayout->setSpacing(8);

    //1) Левая часть: параметры координат (переключаемые)
    m_centralColumn = new QWidget();
    auto* centralLayout = new QVBoxLayout(m_centralColumn);
    centralLayout->setAlignment(Qt::AlignTop);
    centralLayout->setContentsMargins(0, 0, 0, 0);
    centralLayout->setSpacing(2);

    //создание общей панели с параметрами
    m_paramsStack = new QStackedWidget();
    m_cartesianWidgets = new QWidget();
    m_polarWidgets = new QWidget();

    //используем grid layout для декартовых координат (2 поля в ряд)
    auto* cartesianLayout = new QGridLayout(m_cartesianWidgets);
    cartesianLayout->setContentsMargins(0, 2, 0, 0);
    cartesianLayout->setHorizontalSpacing(6);
    cartesianLayout->setVerticalSpacing(2);

    //используем grid layout для полярных координат
    auto* polarLayout = new QGridLayout(m_polarWidgets);
    polarLayout->setContentsMargins(0, 2, 0, 0);
    polarLayout->setHorizontalSpacing(6);
    polarLayout->setVerticalSpacing(2);

    m_paramsStack->addWidget(m_cartesianWidgets);
    m_paramsStack->addWidget(m_polarWidgets);
    centralLayout->addWidget(m_paramsStack);

    //2) Правая часть: цвет + тип линии + слой + кнопка (одна компактная строка)
    m_rightColumn = new QWidget();
    auto* rightLayout = new QHBoxLayout(m_rightColumn);
    rightLayout->setAlignment(Qt::AlignTop);
    rightLayout->setContentsMargins(0, 2, 0, 0);
    rightLayout->setSpacing(8);

    //строка с цветом
    auto* colorLabel = new QLabel("Цвет:");
    m_colorButton = new QPushButton();
    m_colorButton->setObjectName("ColorButton");
    m_colorButton->setFixedSize(24, 24);
    m_colorButton->setCursor(Qt::PointingHandCursor);
    connect(m_colorButton, &QPushButton::clicked, this, &BasePropertiesWidget::onColorButtonClicked);
    rightLayout->addWidget(colorLabel);
    rightLayout->addWidget(m_colorButton);

    //строка с типом линии
    auto* lineLabel = new QLabel("Линия:");
    m_lineTypeComboBox = new QComboBox();
    m_lineTypeComboBox->setFixedWidth(118);
    m_lineTypeComboBox->setIconSize(QSize(92, 16));
    m_lineTypeComboBox->setMinimumContentsLength(0);
    m_lineTypeComboBox->view()->setMinimumWidth(260);
    m_lineTypeComboBox->setObjectName("LineTypeComboBox");
    m_lineTypeComboBox->setCursor(Qt::PointingHandCursor);
    connect(m_lineTypeComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &BasePropertiesWidget::onLineTypeBoxClicked);
    rightLayout->addWidget(lineLabel);
    rightLayout->addWidget(m_lineTypeComboBox);

    //строка со слоем
    auto* layerLabel = new QLabel("Слой:");
    m_layerLineEdit = new QLineEdit("0");
    m_layerLineEdit->setFixedWidth(90);
    connect(m_layerLineEdit, &QLineEdit::editingFinished, this, &BasePropertiesWidget::onLayerNameChanged);
    rightLayout->addWidget(layerLabel);
    rightLayout->addWidget(m_layerLineEdit);

    //кнопка "Создать/Обновить"
    m_applyButton = new QPushButton();
    m_applyButton->setFixedHeight(26);
    m_applyButton->setMinimumWidth(100);
    rightLayout->addWidget(m_applyButton);

    rightLayout->addStretch();

    //сборка главного layout
    mainLayout->addWidget(m_centralColumn, 1);
    mainLayout->addWidget(m_rightColumn, 0);
}

void BasePropertiesWidget::setPrimitives(const QList<BasePrimitive*>& primitives)
{
    m_selectedPrimitives = primitives;

    if (m_selectedPrimitives.isEmpty() || (m_selectedPrimitives.size() == 1 && !m_selectedPrimitives.first())) {
        // Режим создания
        m_currentPrimitive = nullptr;
        m_selectedColor = Qt::white;
        m_selectedLineTypeId = (int)LineType::SolidMain;
        m_selectedLayerName = "0";
        m_applyButton->setText("Создать");
        populateLineTypeComboBox();
        updateColor(m_selectedColor, false);
        updateLineType(m_selectedLineTypeId, false);
        updateLayer(m_selectedLayerName, false);
    }
    else {
        // Режим редактирования
        m_currentPrimitive = m_selectedPrimitives.last();
        m_applyButton->setText(m_selectedPrimitives.size() > 1 ? QString("Обновить (%1)").arg(m_selectedPrimitives.size()) : "Обновить");

        bool diffColors = false;
        bool diffTypes = false;
        bool diffLayers = false;

        QColor firstColor = m_selectedPrimitives.first()->getColor();
        int firstType = m_selectedPrimitives.first()->getLineType();
        QString firstLayer = m_selectedPrimitives.first()->getLayerName();

        for(auto* p : m_selectedPrimitives) {
            if(p->getColor() != firstColor) diffColors = true;
            if(p->getLineType() != firstType) diffTypes = true;
            if(p->getLayerName() != firstLayer) diffLayers = true;
        }

        m_selectedColor = diffColors ? Qt::white : firstColor;
        m_selectedLineTypeId = diffTypes ? -1 : firstType;
        m_selectedLayerName = diffLayers ? "" : firstLayer;

        populateLineTypeComboBox();
        updateColor(m_selectedColor, diffColors);
        updateLineType(m_selectedLineTypeId, diffTypes);
        updateLayer(m_selectedLayerName, diffLayers);
    }

    updateFieldValues();
    applyCompactMetrics();
}

void BasePropertiesWidget::setCoordinateSystem(CoordinateSystemType type)
{
    m_selectedCoordSystem = type;

    if (m_selectedCoordSystem == CoordinateSystemType::Cartesian)
    {
        m_paramsStack->setCurrentWidget(m_cartesianWidgets);
    }
    else
    {
        m_paramsStack->setCurrentWidget(m_polarWidgets);
    }

    updateFieldValues();
    applyCompactMetrics();
}

void BasePropertiesWidget::applyCompactMetrics()
{
    for (auto* edit : findChildren<QLineEdit*>()) {
        if (edit == m_layerLineEdit) continue;
        if (edit->objectName() == "PropertiesInput") {
            edit->setFixedHeight(24);
            edit->setMinimumWidth(58);
            edit->setMaximumWidth(82);
        }
    }

    for (auto* combo : findChildren<QComboBox*>()) {
        if (combo == m_lineTypeComboBox) continue;
        if (combo->objectName() == "PropertiesComboBox") {
            combo->setFixedHeight(24);
            combo->setMinimumWidth(110);
            combo->setMaximumWidth(170);
        }
    }

    for (auto* spin : findChildren<QSpinBox*>()) {
        if (spin->objectName() == "PropertiesInput") {
            spin->setFixedHeight(24);
            spin->setMaximumWidth(82);
        }
    }

    for (auto* spin : findChildren<QDoubleSpinBox*>()) {
        if (spin->objectName() == "PropertiesInput") {
            spin->setFixedHeight(24);
            spin->setMaximumWidth(96);
        }
    }
}

void BasePropertiesWidget::updateColor(const QColor& color, bool isMixed)
{
    m_selectedColor = color;

    if (isMixed) {
        //градиент для индикации разных цветов
        m_colorButton->setStyleSheet("background-color: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 red, stop:0.5 green, stop:1 blue);");
        m_colorButton->setText("?");
    } else {
        //динамический цвет фона
        m_colorButton->setStyleSheet(QString("background-color: %1;").arg(m_selectedColor.name()));
        m_colorButton->setText("");
    }
}

void BasePropertiesWidget::updateLineType(int typeId, bool isMixed)
{
    m_selectedLineTypeId = typeId;

    if (isMixed) {
        if (m_lineTypeComboBox->findText("Разные...") == -1) {
            m_lineTypeComboBox->insertItem(0, "Разные...", -1);
        }
        m_lineTypeComboBox->setCurrentIndex(0);
    } else {
        int index = m_lineTypeComboBox->findData(typeId);
        if (index != -1) {
            m_lineTypeComboBox->setCurrentIndex(index);
        }
    }
}

void BasePropertiesWidget::updateLayer(const QString& name, bool isMixed)
{
    m_selectedLayerName = name;
    if (isMixed) {
        m_layerLineEdit->setText("Разные...");
    } else {
        m_layerLineEdit->setText(name);
    }
}

void BasePropertiesWidget::onColorButtonClicked()
{
    QColor color = QColorDialog::getColor(m_selectedColor, this, "Выберите цвет");
    if (color.isValid()) {
        m_selectedColor = color;
        updateColor(m_selectedColor, false);
        emit colorChanged(m_selectedColor);
    }
}

void BasePropertiesWidget::onLineTypeBoxClicked(int index)
{
    if (index < 0) return;

    int selectedTypeId = m_lineTypeComboBox->itemData(index).toInt();

    if (selectedTypeId == -1) return;

    m_selectedLineTypeId = selectedTypeId;

    if (!m_currentPrimitive && m_selectedPrimitives.isEmpty()) {
        emit lineTypeChanged(static_cast<LineType>(m_selectedLineTypeId));
    }
}

void BasePropertiesWidget::onLayerNameChanged()
{
    QString name = m_layerLineEdit->text();
    if (name == "Разные..." || name.trimmed().isEmpty()) {
        name = "0";
        m_layerLineEdit->setText(name);
    }
    m_selectedLayerName = name;
    
    emit layerChanged(m_selectedLayerName);
}

void BasePropertiesWidget::populateLineTypeComboBox()
{
    m_lineTypeComboBox->blockSignals(true);
    m_lineTypeComboBox->clear();
    QColor iconColor = ThemeManager::instance().getIconColor();

    QMap<LineType, QString> lineTypeIcons;
    lineTypeIcons[LineType::SolidMain]      = ":/icons/icons/lines/line-solid-main.svg";
    lineTypeIcons[LineType::SolidThin]      = ":/icons/icons/lines/line-solid-thin.svg";
    lineTypeIcons[LineType::SolidWave]      = ":/icons/icons/lines/line-wavy.svg";
    lineTypeIcons[LineType::SolidKink]      = ":/icons/icons/lines/line-zigzag.svg";
    lineTypeIcons[LineType::Dashed]         = ":/icons/icons/lines/line-dashed.svg";
    lineTypeIcons[LineType::DashDotThick]   = ":/icons/icons/lines/line-dashdot-thick.svg";
    lineTypeIcons[LineType::DashDotThin]    = ":/icons/icons/lines/line-dashdot-thin.svg";
    lineTypeIcons[LineType::DashDotDot]     = ":/icons/icons/lines/line-dashdotdot.svg";

    for (auto it = lineTypeIcons.constBegin(); it != lineTypeIcons.constEnd(); ++it)
    {
        LineType type = it.key();
        QString path = it.value();
        QIcon icon = ThemeManager::colorizeSvg(path, iconColor);

        QString name;
        switch(type) {
        case LineType::SolidMain: name = "Сплошная"; break;
        case LineType::SolidThin: name = "Тонкая"; break;
        case LineType::SolidWave: name = "Волнистая"; break;
        case LineType::Dashed:    name = "Штриховая"; break;
        case LineType::DashDotThick: name = "Штрих-пункт."; break;
        case LineType::DashDotThin:  name = "Штрих-пункт. тон."; break;
        case LineType::DashDotDot:   name = "Две точки"; break;
        case LineType::SolidKink:    name = "С изломами"; break;
        default: name = "Линия"; break;
        }

        m_lineTypeComboBox->addItem(icon, name, static_cast<int>(type));
        m_lineTypeComboBox->setItemData(m_lineTypeComboBox->count() - 1, name, Qt::ToolTipRole);
    }

    auto customStyles = LineStyleManager::instance().getCustomStyles();
    for(auto it = customStyles.begin(); it != customStyles.end(); ++it) {
        QPixmap pix(80, 16);
        pix.fill(Qt::transparent);
        QPainter p(&pix);
        LineStyleManager::instance().drawLine(p, QPointF(0, 8), QPointF(80, 8), it.key(), iconColor);

        m_lineTypeComboBox->addItem(QIcon(pix), it.value().name, it.key());
        m_lineTypeComboBox->setItemData(m_lineTypeComboBox->count() - 1, it.value().name, Qt::ToolTipRole);
    }

    m_lineTypeComboBox->blockSignals(false);
}

void BasePropertiesWidget::updateColors()
{
    populateLineTypeComboBox();
}
