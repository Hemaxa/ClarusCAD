#include "SettingsWindow.h"
#include "SettingsManager.h"
#include "LineStyleManager.h"

#include <QComboBox>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QDialogButtonBox>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QLabel>
#include <QTabWidget>
#include <QPushButton>
#include <QListWidget>
#include <QLineEdit>
#include <QMessageBox>
#include <QCheckBox>
#include <QScrollArea>
#include <QColorDialog>
#include <QFontComboBox>

SettingsWindow::SettingsWindow(QWidget* parent) : QDialog(parent)
{
    //настройки окна
    setWindowTitle("Настройки");
    setMinimumWidth(580);
    setMinimumHeight(850); // Увеличили для стилей линий

    //создание табов
    m_tabWidget = new QTabWidget();
    m_tabWidget->setTabPosition(QTabWidget::South); //Вкладки снизу

    //стилизация табов вынесена в файл темы ClarusCAD.qss

    m_tabWidget->addTab(createAppearanceTab(), "Оформление");
    m_tabWidget->addTab(createViewportTab(), "Рабочая область");
    m_tabWidget->addTab(createLineStylesTab(), "Стили линий");
    m_tabWidget->addTab(createDimensionStylesTab(), "Стили размеров");

    //кнопки "OK" и "Отмена"
    auto* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &SettingsWindow::applySettings);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    //сборка всего диалогового окна вместе
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(15, 15, 15, 15);
    mainLayout->setSpacing(15);
    mainLayout->addWidget(m_tabWidget);
    mainLayout->addWidget(buttonBox);
}

QWidget* SettingsWindow::createAppearanceTab()
{
    auto* widget = new QWidget();
    auto* mainLayout = new QVBoxLayout(widget);
    mainLayout->setContentsMargins(15, 15, 15, 15);

    //Оборачиваем в GroupBox для зеленой рамки
    auto* groupBox = new QGroupBox("Настройки интерфейса");
    auto* formLayout = new QFormLayout(groupBox);
    formLayout->setSpacing(15);
    formLayout->setContentsMargins(15, 25, 15, 15); //padding-top побольше для заголовка

    m_themeComboBox = new QComboBox();
    m_themeComboBox->setFixedHeight(30);
    populateThemeComboBox();

    //установка текущего
    int index = m_themeComboBox->findData(SettingsManager::instance().getThemeName());
    if (index != -1) m_themeComboBox->setCurrentIndex(index);

    formLayout->addRow("Тема оформления:", m_themeComboBox);

    mainLayout->addWidget(groupBox);

    mainLayout->addStretch(); //Прижать вверх
    return widget;
}

QWidget* SettingsWindow::createDimensionStylesTab()
{
    auto* scrollArea = new QScrollArea();
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);

    auto* widget = new QWidget();
    auto* mainLayout = new QVBoxLayout(widget);
    mainLayout->setContentsMargins(15, 15, 15, 15);
    mainLayout->setSpacing(10);

    auto* textGroup = new QGroupBox("Текст размера");
    auto* textLayout = new QFormLayout(textGroup);
    textLayout->setSpacing(10);
    textLayout->setContentsMargins(15, 25, 15, 15);

    auto* extensionGroup = new QGroupBox("Выносные линии");
    auto* extensionLayout = new QFormLayout(extensionGroup);
    extensionLayout->setSpacing(10);
    extensionLayout->setContentsMargins(15, 25, 15, 15);

    auto* dimensionLineGroup = new QGroupBox("Размерная линия");
    auto* dimensionLineLayout = new QFormLayout(dimensionLineGroup);
    dimensionLineLayout->setSpacing(10);
    dimensionLineLayout->setContentsMargins(15, 25, 15, 15);

    auto* arrowGroup = new QGroupBox("Стрелки");
    auto* arrowLayout = new QFormLayout(arrowGroup);
    arrowLayout->setSpacing(10);
    arrowLayout->setContentsMargins(15, 25, 15, 15);

    auto populateLineTypes = [](QComboBox* combo) {
        combo->clear();
        combo->addItem("Сплошная", static_cast<int>(LineType::SolidMain));
        combo->addItem("Тонкая", static_cast<int>(LineType::SolidThin));
        combo->addItem("Штриховая", static_cast<int>(LineType::Dashed));
        combo->addItem("Штрих-пунктирная", static_cast<int>(LineType::DashDotThin));
        combo->addItem("Две точки", static_cast<int>(LineType::DashDotDot));
    };
    auto setButtonColor = [](QPushButton* button, const QColor& color) {
        button->setStyleSheet(QString("background-color: %1;").arg(color.name()));
        button->setText(color.name());
    };

    m_dimensionFontComboBox = new QFontComboBox();
    m_dimensionFontComboBox->setCurrentFont(QFont(SettingsManager::instance().getDimensionFontFamily()));

    m_dimensionTextHeightSpinBox = new QDoubleSpinBox();
    m_dimensionTextHeightSpinBox->setRange(6.0, 48.0);
    m_dimensionTextHeightSpinBox->setValue(SettingsManager::instance().getDimensionTextHeight());
    m_dimensionTextHeightSpinBox->setSuffix(" px");

    m_dimensionTextGapSpinBox = new QDoubleSpinBox();
    m_dimensionTextGapSpinBox->setRange(0.0, 40.0);
    m_dimensionTextGapSpinBox->setValue(SettingsManager::instance().getDimensionTextGap());
    m_dimensionTextGapSpinBox->setSuffix(" px");

    m_dimensionArrowSizeSpinBox = new QDoubleSpinBox();
    m_dimensionArrowSizeSpinBox->setRange(4.0, 40.0);
    m_dimensionArrowSizeSpinBox->setValue(SettingsManager::instance().getDimensionArrowSize());
    m_dimensionArrowSizeSpinBox->setSuffix(" px");

    m_dimensionArrowTypeComboBox = new QComboBox();
    m_dimensionArrowTypeComboBox->addItem("Классическая (закрытая)", static_cast<int>(DimensionArrowType::ClosedFilled));
    m_dimensionArrowTypeComboBox->addItem("Разомкнутая (открытая)", static_cast<int>(DimensionArrowType::ClosedOpen));
    m_dimensionArrowTypeComboBox->addItem("Засечка", static_cast<int>(DimensionArrowType::Slash));
    m_dimensionArrowTypeComboBox->addItem("Точка", static_cast<int>(DimensionArrowType::Dot));
    m_dimensionArrowTypeComboBox->setCurrentIndex(m_dimensionArrowTypeComboBox->findData(static_cast<int>(SettingsManager::instance().getDimensionArrowType())));

    m_dimensionArrowFilledCheck = new QCheckBox("Заполненные стрелки");
    m_dimensionArrowFilledCheck->setChecked(SettingsManager::instance().getDimensionArrowFilled());

    m_dimensionExtOffsetSpinBox = new QDoubleSpinBox();
    m_dimensionExtOffsetSpinBox->setRange(0.0, 50.0);
    m_dimensionExtOffsetSpinBox->setValue(SettingsManager::instance().getDimensionExtensionOffset());
    m_dimensionExtOffsetSpinBox->setSuffix(" px");

    m_dimensionExtExtendSpinBox = new QDoubleSpinBox();
    m_dimensionExtExtendSpinBox->setRange(0.0, 50.0);
    m_dimensionExtExtendSpinBox->setValue(SettingsManager::instance().getDimensionExtensionExtend());
    m_dimensionExtExtendSpinBox->setSuffix(" px");

    m_dimensionLineExtendSpinBox = new QDoubleSpinBox();
    m_dimensionLineExtendSpinBox->setRange(0.0, 50.0);
    m_dimensionLineExtendSpinBox->setValue(SettingsManager::instance().getDimensionLineExtension());
    m_dimensionLineExtendSpinBox->setSuffix(" px");

    m_dimensionExtLineTypeComboBox = new QComboBox();
    populateLineTypes(m_dimensionExtLineTypeComboBox);
    m_dimensionExtLineTypeComboBox->setCurrentIndex(m_dimensionExtLineTypeComboBox->findData(SettingsManager::instance().getDimensionExtensionLineType()));

    m_dimensionLineTypeComboBox = new QComboBox();
    populateLineTypes(m_dimensionLineTypeComboBox);
    m_dimensionLineTypeComboBox->setCurrentIndex(m_dimensionLineTypeComboBox->findData(SettingsManager::instance().getDimensionLineType()));

    m_dimensionTextColorButton = new QPushButton();
    m_dimensionExtColorButton = new QPushButton();
    m_dimensionLineColorButton = new QPushButton();
    m_dimensionTextColorButton->setProperty("selectedColor", SettingsManager::instance().getDimensionTextColor());
    m_dimensionExtColorButton->setProperty("selectedColor", SettingsManager::instance().getDimensionExtensionLineColor());
    m_dimensionLineColorButton->setProperty("selectedColor", SettingsManager::instance().getDimensionLineColor());
    setButtonColor(m_dimensionTextColorButton, SettingsManager::instance().getDimensionTextColor());
    setButtonColor(m_dimensionExtColorButton, SettingsManager::instance().getDimensionExtensionLineColor());
    setButtonColor(m_dimensionLineColorButton, SettingsManager::instance().getDimensionLineColor());

    auto attachColorPicker = [this, setButtonColor](QPushButton* button) {
        connect(button, &QPushButton::clicked, this, [button, setButtonColor]() {
            QColor initial = button->property("selectedColor").value<QColor>();
            QColor color = QColorDialog::getColor(initial, button);
            if (color.isValid()) {
                button->setProperty("selectedColor", color);
                setButtonColor(button, color);
            }
        });
    };
    attachColorPicker(m_dimensionTextColorButton);
    attachColorPicker(m_dimensionExtColorButton);
    attachColorPicker(m_dimensionLineColorButton);

    textLayout->addRow("Шрифт:", m_dimensionFontComboBox);
    textLayout->addRow("Высота:", m_dimensionTextHeightSpinBox);
    textLayout->addRow("Отступ от линии:", m_dimensionTextGapSpinBox);
    textLayout->addRow("Цвет:", m_dimensionTextColorButton);

    extensionLayout->addRow("Цвет:", m_dimensionExtColorButton);
    extensionLayout->addRow("Тип линии:", m_dimensionExtLineTypeComboBox);
    extensionLayout->addRow("Отступ от объекта:", m_dimensionExtOffsetSpinBox);
    extensionLayout->addRow("Выход за размерную:", m_dimensionExtExtendSpinBox);

    dimensionLineLayout->addRow("Цвет:", m_dimensionLineColorButton);
    dimensionLineLayout->addRow("Тип линии:", m_dimensionLineTypeComboBox);
    dimensionLineLayout->addRow("Расширение за выносные:", m_dimensionLineExtendSpinBox);

    arrowLayout->addRow("Тип:", m_dimensionArrowTypeComboBox);
    arrowLayout->addRow("Размер:", m_dimensionArrowSizeSpinBox);
    arrowLayout->addRow("", m_dimensionArrowFilledCheck);

    mainLayout->addWidget(textGroup);
    mainLayout->addWidget(extensionGroup);
    mainLayout->addWidget(dimensionLineGroup);
    mainLayout->addWidget(arrowGroup);
    mainLayout->addStretch();

    scrollArea->setWidget(widget);
    return scrollArea;
}

QWidget* SettingsWindow::createViewportTab()
{
    auto* widget = new QWidget();
    auto* mainLayout = new QVBoxLayout(widget);
    mainLayout->setContentsMargins(15, 15, 15, 15);

    //Оборачиваем в GroupBox для зеленой рамки
    auto* groupBox = new QGroupBox("Параметры сцены");
    auto* formLayout = new QFormLayout(groupBox);
    formLayout->setSpacing(15);
    formLayout->setContentsMargins(15, 25, 15, 15);

    //настройка шага сетки
    m_gridStepSpinBox = new QSpinBox();
    m_gridStepSpinBox->setRange(10, 200);
    m_gridStepSpinBox->setSingleStep(5);
    m_gridStepSpinBox->setSuffix(" px");
    m_gridStepSpinBox->setFixedHeight(30);
    m_gridStepSpinBox->setValue(SettingsManager::instance().getGridStep());

    //настройка шага увеличения/уменьшения
    m_zoomStepSpinBox = new QDoubleSpinBox();
    m_zoomStepSpinBox->setRange(1.10, 3.00);
    m_zoomStepSpinBox->setSingleStep(0.05);
    m_zoomStepSpinBox->setDecimals(2);
    m_zoomStepSpinBox->setSuffix("x");
    m_zoomStepSpinBox->setFixedHeight(30);
    m_zoomStepSpinBox->setValue(SettingsManager::instance().getZoomStep());

    //настройка единиц измерения углов
    m_angleUnitComboBox = new QComboBox();
    m_angleUnitComboBox->addItem("Градусы", static_cast<int>(AngleUnit::Degrees));
    m_angleUnitComboBox->addItem("Радианы", static_cast<int>(AngleUnit::Radians));
    m_angleUnitComboBox->setFixedHeight(30);
    int unitIndex = m_angleUnitComboBox->findData(static_cast<int>(SettingsManager::instance().getAngleUnit()));
    if (unitIndex != -1) m_angleUnitComboBox->setCurrentIndex(unitIndex);

    formLayout->addRow("Шаг сетки:", m_gridStepSpinBox);
    formLayout->addRow("Шаг зума:", m_zoomStepSpinBox);
    formLayout->addRow("Единицы углов:", m_angleUnitComboBox);

    mainLayout->addWidget(groupBox);
    mainLayout->addStretch();
    return widget;
}

QWidget* SettingsWindow::createLineStylesTab()
{
    // Создаём ScrollArea для прокрутки содержимого
    auto* scrollArea = new QScrollArea();
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    
    auto* contentWidget = new QWidget();
    auto* mainLayout = new QVBoxLayout(contentWidget);
    mainLayout->setSpacing(10);
    mainLayout->setContentsMargins(15, 15, 15, 15);

    // =======================================================
    // 1. Группа основных настроек
    // =======================================================
    auto* baseGroup = new QGroupBox("Общие параметры линий");
    auto* baseLayout = new QFormLayout(baseGroup);
    baseLayout->setContentsMargins(15, 20, 15, 10);

    m_lineThicknessSpinBox = new QDoubleSpinBox();
    m_lineThicknessSpinBox->setRange(0.5, 20.0);
    m_lineThicknessSpinBox->setSingleStep(0.1);
    m_lineThicknessSpinBox->setSuffix(" px");
    m_lineThicknessSpinBox->setFixedHeight(28);
    m_lineThicknessSpinBox->setValue(SettingsManager::instance().getBaseLineThickness());

    m_dashLengthSpinBox = new QDoubleSpinBox();
    m_dashLengthSpinBox->setRange(1.0, 100.0);
    m_dashLengthSpinBox->setSingleStep(1.0);
    m_dashLengthSpinBox->setSuffix(" px");
    m_dashLengthSpinBox->setFixedHeight(28);
    m_dashLengthSpinBox->setValue(SettingsManager::instance().getDashLength());

    m_dashSpaceSpinBox = new QDoubleSpinBox();
    m_dashSpaceSpinBox->setRange(1.0, 100.0);
    m_dashSpaceSpinBox->setSingleStep(1.0);
    m_dashSpaceSpinBox->setSuffix(" px");
    m_dashSpaceSpinBox->setFixedHeight(28);
    m_dashSpaceSpinBox->setValue(SettingsManager::instance().getDashSpace());

    baseLayout->addRow("Базовая толщина:", m_lineThicknessSpinBox);
    baseLayout->addRow("Длина штриха:", m_dashLengthSpinBox);
    baseLayout->addRow("Расстояние:", m_dashSpaceSpinBox);

    mainLayout->addWidget(baseGroup);

    // =======================================================
    // 2. Группа волнистой линии
    // =======================================================
    auto* waveGroup = new QGroupBox("Волнистая линия");
    auto* waveLayout = new QFormLayout(waveGroup);
    waveLayout->setContentsMargins(15, 20, 15, 10);

    m_waveAmplitudeSpinBox = new QDoubleSpinBox();
    m_waveAmplitudeSpinBox->setRange(0.5, 20.0);
    m_waveAmplitudeSpinBox->setSingleStep(0.5);
    m_waveAmplitudeSpinBox->setSuffix(" px");
    m_waveAmplitudeSpinBox->setFixedHeight(28);
    m_waveAmplitudeSpinBox->setValue(SettingsManager::instance().getWaveAmplitude());

    m_wavePeriodSpinBox = new QDoubleSpinBox();
    m_wavePeriodSpinBox->setRange(2.0, 100.0);
    m_wavePeriodSpinBox->setSingleStep(1.0);
    m_wavePeriodSpinBox->setSuffix(" px");
    m_wavePeriodSpinBox->setFixedHeight(28);
    m_wavePeriodSpinBox->setValue(SettingsManager::instance().getWavePeriod());

    waveLayout->addRow("Амплитуда:", m_waveAmplitudeSpinBox);
    waveLayout->addRow("Период:", m_wavePeriodSpinBox);

    mainLayout->addWidget(waveGroup);

    // =======================================================
    // 3. Группа линии с изломами
    // =======================================================
    auto* kinkGroup = new QGroupBox("Линия с изломами");
    auto* kinkLayout = new QFormLayout(kinkGroup);
    kinkLayout->setContentsMargins(15, 20, 15, 10);

    m_kinkAmplitudeSpinBox = new QDoubleSpinBox();
    m_kinkAmplitudeSpinBox->setRange(0.5, 20.0);
    m_kinkAmplitudeSpinBox->setSingleStep(0.5);
    m_kinkAmplitudeSpinBox->setSuffix(" px");
    m_kinkAmplitudeSpinBox->setFixedHeight(28);
    m_kinkAmplitudeSpinBox->setValue(SettingsManager::instance().getKinkAmplitude());

    m_kinkLengthSpinBox = new QDoubleSpinBox();
    m_kinkLengthSpinBox->setRange(1.0, 50.0);
    m_kinkLengthSpinBox->setSingleStep(1.0);
    m_kinkLengthSpinBox->setSuffix(" px");
    m_kinkLengthSpinBox->setFixedHeight(28);
    m_kinkLengthSpinBox->setValue(SettingsManager::instance().getKinkLength());

    m_kinkStraightSpinBox = new QDoubleSpinBox();
    m_kinkStraightSpinBox->setRange(1.0, 100.0);
    m_kinkStraightSpinBox->setSingleStep(1.0);
    m_kinkStraightSpinBox->setSuffix(" px");
    m_kinkStraightSpinBox->setFixedHeight(28);
    m_kinkStraightSpinBox->setValue(SettingsManager::instance().getKinkStraight());

    kinkLayout->addRow("Амплитуда:", m_kinkAmplitudeSpinBox);
    kinkLayout->addRow("Длина излома:", m_kinkLengthSpinBox);
    kinkLayout->addRow("Прямой участок:", m_kinkStraightSpinBox);

    mainLayout->addWidget(kinkGroup);

    // =======================================================
    // 4. Группа пользовательских стилей
    // =======================================================
    auto* customGroup = new QGroupBox("Конструктор стилей");
    auto* customLayout = new QVBoxLayout(customGroup);
    customLayout->setContentsMargins(15, 20, 15, 10);
    customLayout->setSpacing(8);

    m_stylesListWidget = new QListWidget();
    m_stylesListWidget->setFixedHeight(80);
    connect(m_stylesListWidget, &QListWidget::itemSelectionChanged, this, &SettingsWindow::onStyleSelectionChanged);

    // Заполняем список
    auto styles = LineStyleManager::instance().getCustomStyles();
    for (auto it = styles.begin(); it != styles.end(); ++it) {
        QListWidgetItem* item = new QListWidgetItem(it.value().name);
        item->setData(Qt::UserRole, it.key());
        m_stylesListWidget->addItem(item);
    }

    // Поле имени
    m_styleNameEdit = new QLineEdit();
    m_styleNameEdit->setPlaceholderText("Название стиля");
    m_styleNameEdit->setFixedHeight(28);

    // Кнопки конструктора паттерна
    auto* builderLayout = new QHBoxLayout();
    auto* addDashBtn = new QPushButton("Штрих");
    auto* addSpaceBtn = new QPushButton("Пробел");
    auto* addDotBtn = new QPushButton("Точка");
    auto* clearBtn = new QPushButton("Очистить");

    addDashBtn->setToolTip("Добавить штрих (10px)");
    addDotBtn->setToolTip("Добавить точку (2px)");
    addSpaceBtn->setToolTip("Добавить пробел (5px)");

    connect(addDashBtn, &QPushButton::clicked, this, &SettingsWindow::onAddDash);
    connect(addSpaceBtn, &QPushButton::clicked, this, &SettingsWindow::onAddSpace);
    connect(addDotBtn, &QPushButton::clicked, this, &SettingsWindow::onAddDot);
    connect(clearBtn, &QPushButton::clicked, this, &SettingsWindow::onClearPattern);

    builderLayout->addWidget(addDashBtn);
    builderLayout->addWidget(addSpaceBtn);
    builderLayout->addWidget(addDotBtn);
    builderLayout->addWidget(clearBtn);

    m_patternPreviewLabel = new QLabel("Шаблон: (пусто)");
    m_patternPreviewLabel->setObjectName("PatternPreviewLabel");

    // Индивидуальная толщина
    auto* thicknessLayout = new QHBoxLayout();
    m_useCustomThicknessCheck = new QCheckBox("Своя толщина:");
    m_customThicknessSpinBox = new QDoubleSpinBox();
    m_customThicknessSpinBox->setRange(0.5, 20.0);
    m_customThicknessSpinBox->setSingleStep(0.5);
    m_customThicknessSpinBox->setSuffix(" px");
    m_customThicknessSpinBox->setValue(2.0);
    m_customThicknessSpinBox->setEnabled(false);
    connect(m_useCustomThicknessCheck, &QCheckBox::toggled, m_customThicknessSpinBox, &QDoubleSpinBox::setEnabled);
    thicknessLayout->addWidget(m_useCustomThicknessCheck);
    thicknessLayout->addWidget(m_customThicknessSpinBox);
    thicknessLayout->addStretch();

    // Кнопки управления
    auto* actionLayout = new QHBoxLayout();
    auto* addStyleBtn = new QPushButton("Сохранить");
    auto* editStyleBtn = new QPushButton("Редактировать");
    auto* delStyleBtn = new QPushButton("Удалить");

    connect(addStyleBtn, &QPushButton::clicked, this, &SettingsWindow::onAddStyleClicked);
    connect(editStyleBtn, &QPushButton::clicked, this, &SettingsWindow::onEditStyleClicked);
    connect(delStyleBtn, &QPushButton::clicked, this, &SettingsWindow::onDeleteStyleClicked);

    actionLayout->addWidget(addStyleBtn);
    actionLayout->addWidget(editStyleBtn);
    actionLayout->addWidget(delStyleBtn);

    customLayout->addWidget(m_stylesListWidget);
    customLayout->addWidget(m_styleNameEdit);
    customLayout->addLayout(builderLayout);
    customLayout->addWidget(m_patternPreviewLabel);
    customLayout->addLayout(thicknessLayout);
    customLayout->addLayout(actionLayout);

    mainLayout->addWidget(customGroup);
    mainLayout->addStretch(); // Прижать содержимое к верху

    scrollArea->setWidget(contentWidget);
    return scrollArea;
}

// --- Логика конструктора паттерна ---

void SettingsWindow::onAddDash()
{
    m_currentPattern.append(10.0); // Длина штриха
    updatePatternPreview();
}

void SettingsWindow::onAddDot()
{
    m_currentPattern.append(2.0); // Длина точки
    updatePatternPreview();
}

void SettingsWindow::onAddSpace()
{
    m_currentPattern.append(5.0); // Длина пробела
    updatePatternPreview();
}

void SettingsWindow::onClearPattern()
{
    m_currentPattern.clear();
    updatePatternPreview();
}

void SettingsWindow::updatePatternPreview()
{
    if (m_currentPattern.isEmpty()) {
        m_patternPreviewLabel->setText("Шаблон: (пусто)");
        return;
    }

    QStringList parts;
    for(int i = 0; i < m_currentPattern.size(); ++i) {
        qreal val = m_currentPattern[i];
        QString name;
        if (val == 10.0) name = "Штрих";
        else if (val == 2.0) name = "Точка";
        else if (val == 5.0) name = "Пробел";
        else name = QString::number(val);

        parts << name;
    }
    m_patternPreviewLabel->setText("Шаблон: " + parts.join(" - "));
}

void SettingsWindow::onAddStyleClicked()
{
    QString name = m_styleNameEdit->text().trimmed();

    if (name.isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Введите название стиля.");
        return;
    }

    if (m_currentPattern.isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Шаблон пуст. Добавьте элементы (штрих, точка...).");
        return;
    }

    if (m_currentPattern.size() % 2 != 0) {
        m_currentPattern.append(5.0);
    }

    // Определяем толщину
    double thickness = m_useCustomThicknessCheck->isChecked() ? m_customThicknessSpinBox->value() : -1.0;

    if (m_editingStyleId >= 0) {
        // Редактирование существующего
        LineStyleManager::instance().updateCustomStyle(m_editingStyleId, name, m_currentPattern, thickness);
        
        // Обновим текст в списке
        for (int i = 0; i < m_stylesListWidget->count(); ++i) {
            auto* item = m_stylesListWidget->item(i);
            if (item->data(Qt::UserRole).toInt() == m_editingStyleId) {
                item->setText(name);
                break;
            }
        }
        m_editingStyleId = -1;
    } else {
        // Создание нового
        int newId = LineStyleManager::instance().generateNewId();
        LineStyleManager::instance().addCustomStyle(newId, name, m_currentPattern, thickness);

        QListWidgetItem* item = new QListWidgetItem(name);
        item->setData(Qt::UserRole, newId);
        m_stylesListWidget->addItem(item);
    }

    m_styleNameEdit->clear();
    m_useCustomThicknessCheck->setChecked(false);
    onClearPattern();
}

void SettingsWindow::onEditStyleClicked()
{
    auto items = m_stylesListWidget->selectedItems();
    if (items.isEmpty()) {
        QMessageBox::information(this, "Редактирование", "Выберите стиль для редактирования.");
        return;
    }

    auto* item = items.first();
    int id = item->data(Qt::UserRole).toInt();
    auto styles = LineStyleManager::instance().getCustomStyles();

    if (!styles.contains(id)) return;

    const CustomLineStyle& style = styles[id];
    m_editingStyleId = id;
    m_styleNameEdit->setText(style.name);
    m_currentPattern = style.pattern;
    
    if (style.thickness > 0) {
        m_useCustomThicknessCheck->setChecked(true);
        m_customThicknessSpinBox->setValue(style.thickness);
    } else {
        m_useCustomThicknessCheck->setChecked(false);
    }
    
    updatePatternPreview();
}

void SettingsWindow::onStyleSelectionChanged()
{
    // Сбрасываем режим редактирования при смене выделения
    // (опционально, можно убрать если не нужно)
}

void SettingsWindow::onDeleteStyleClicked()
{
    auto items = m_stylesListWidget->selectedItems();
    if (items.isEmpty()) return;

    for (auto* item : items) {
        int id = item->data(Qt::UserRole).toInt();
        LineStyleManager::instance().removeCustomStyle(id);
        delete m_stylesListWidget->takeItem(m_stylesListWidget->row(item));
    }
    m_editingStyleId = -1;
}

void SettingsWindow::applySettings()
{
    //получение новых значений из UI
    QString selectedTheme = m_themeComboBox->currentData().toString();
    int newGridStep = m_gridStepSpinBox->value();
    double newZoomStep = m_zoomStepSpinBox->value();
    AngleUnit newAngleUnit = static_cast<AngleUnit>(m_angleUnitComboBox->currentData().toInt());
    
    // Базовые параметры линий
    double newThickness = m_lineThicknessSpinBox->value();
    double newDashLength = m_dashLengthSpinBox->value();
    double newDashSpace = m_dashSpaceSpinBox->value();
    
    // Параметры волнистой линии
    double newWaveAmplitude = m_waveAmplitudeSpinBox->value();
    double newWavePeriod = m_wavePeriodSpinBox->value();
    
    // Параметры линии с изломами
    double newKinkAmplitude = m_kinkAmplitudeSpinBox->value();
    double newKinkLength = m_kinkLengthSpinBox->value();
    double newKinkStraight = m_kinkStraightSpinBox->value();

    //отправка значений в SettingsManager
    SettingsManager::instance().setThemeName(selectedTheme);
    SettingsManager::instance().setGridStep(newGridStep);
    SettingsManager::instance().setZoomStep(newZoomStep);
    SettingsManager::instance().setAngleUnit(newAngleUnit);
    SettingsManager::instance().setBaseLineThickness(newThickness);
    SettingsManager::instance().setDashLength(newDashLength);
    SettingsManager::instance().setDashSpace(newDashSpace);
    SettingsManager::instance().setWaveAmplitude(newWaveAmplitude);
    SettingsManager::instance().setWavePeriod(newWavePeriod);
    SettingsManager::instance().setKinkAmplitude(newKinkAmplitude);
    SettingsManager::instance().setKinkLength(newKinkLength);
    SettingsManager::instance().setKinkStraight(newKinkStraight);
    SettingsManager::instance().setDimensionFontFamily(m_dimensionFontComboBox->currentFont().family());
    SettingsManager::instance().setDimensionTextHeight(m_dimensionTextHeightSpinBox->value());
    SettingsManager::instance().setDimensionTextGap(m_dimensionTextGapSpinBox->value());
    SettingsManager::instance().setDimensionArrowSize(m_dimensionArrowSizeSpinBox->value());
    SettingsManager::instance().setDimensionArrowType(static_cast<DimensionArrowType>(m_dimensionArrowTypeComboBox->currentData().toInt()));
    SettingsManager::instance().setDimensionArrowFilled(m_dimensionArrowFilledCheck->isChecked());
    SettingsManager::instance().setDimensionExtensionOffset(m_dimensionExtOffsetSpinBox->value());
    SettingsManager::instance().setDimensionExtensionExtend(m_dimensionExtExtendSpinBox->value());
    SettingsManager::instance().setDimensionLineExtension(m_dimensionLineExtendSpinBox->value());
    SettingsManager::instance().setDimensionExtensionLineType(m_dimensionExtLineTypeComboBox->currentData().toInt());
    SettingsManager::instance().setDimensionLineType(m_dimensionLineTypeComboBox->currentData().toInt());
    SettingsManager::instance().setDimensionTextColor(m_dimensionTextColorButton->property("selectedColor").value<QColor>());
    SettingsManager::instance().setDimensionExtensionLineColor(m_dimensionExtColorButton->property("selectedColor").value<QColor>());
    SettingsManager::instance().setDimensionLineColor(m_dimensionLineColorButton->property("selectedColor").value<QColor>());

    //сохранение значений в SettingsManager
    SettingsManager::instance().saveSettings();
}

void SettingsWindow::populateThemeComboBox()
{
    //добавление тем
    m_themeComboBox->addItem("ClarusCAD", "ClarusCAD");
    m_themeComboBox->addItem("Hello Kitty", "HelloKitty");
    m_themeComboBox->addItem("Темная", "Dark");
    m_themeComboBox->addItem("Светлая", "Light");
}
