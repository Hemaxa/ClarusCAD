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
#include <QRegularExpression>

SettingsWindow::SettingsWindow(QWidget* parent) : QDialog(parent)
{
    //настройки окна
    setWindowTitle("Настройки");
    setMinimumWidth(550);
    setMinimumHeight(550); //Чуть увеличили высоту

    //создание табов
    m_tabWidget = new QTabWidget();
    m_tabWidget->setTabPosition(QTabWidget::South); //Вкладки снизу

    //Стилизация табов "в тему" приложения (зеленый акцент)
    m_tabWidget->setStyleSheet(
        "QTabWidget::pane { border: 1px solid #3c3c3c; border-bottom: 1px solid #00ff7f; }"
        "QTabBar::tab { background: #2a2a2a; color: #e0e0e0; padding: 8px 12px; border: 1px solid #3c3c3c; margin-right: 2px; }"
        "QTabBar::tab:selected { background: #1e1e1e; color: #00ff7f; border-color: #00ff7f; border-top: 1px solid #1e1e1e; }"
        "QTabBar::tab:hover { background: #3c3c3c; }"
        );

    m_tabWidget->addTab(createAppearanceTab(), "Оформление");
    m_tabWidget->addTab(createViewportTab(), "Рабочая область");
    m_tabWidget->addTab(createLineStylesTab(), "Стили линий");

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
    auto* widget = new QWidget();
    auto* mainLayout = new QVBoxLayout(widget);
    mainLayout->setSpacing(15);
    mainLayout->setContentsMargins(15, 15, 15, 15);

    // 1. Группа основных настроек
    auto* baseGroup = new QGroupBox("Общие параметры линий");
    auto* baseLayout = new QFormLayout(baseGroup);
    baseLayout->setContentsMargins(15, 25, 15, 15);

    m_lineThicknessSpinBox = new QDoubleSpinBox();
    m_lineThicknessSpinBox->setRange(0.5, 20.0);
    m_lineThicknessSpinBox->setSingleStep(0.5);
    m_lineThicknessSpinBox->setSuffix(" px");
    m_lineThicknessSpinBox->setFixedHeight(30);
    m_lineThicknessSpinBox->setValue(SettingsManager::instance().getBaseLineThickness());

    // Настройка длины штриха
    m_dashLengthSpinBox = new QDoubleSpinBox();
    m_dashLengthSpinBox->setRange(1.0, 100.0);
    m_dashLengthSpinBox->setSingleStep(1.0);
    m_dashLengthSpinBox->setSuffix(" px");
    m_dashLengthSpinBox->setFixedHeight(30);
    m_dashLengthSpinBox->setValue(SettingsManager::instance().getDashLength());

    // Настройка расстояния
    m_dashSpaceSpinBox = new QDoubleSpinBox();
    m_dashSpaceSpinBox->setRange(1.0, 100.0);
    m_dashSpaceSpinBox->setSingleStep(1.0);
    m_dashSpaceSpinBox->setSuffix(" px");
    m_dashSpaceSpinBox->setFixedHeight(30);
    m_dashSpaceSpinBox->setValue(SettingsManager::instance().getDashSpace());

    baseLayout->addRow("Базовая толщина:", m_lineThicknessSpinBox);
    baseLayout->addRow("Длина штриха:", m_dashLengthSpinBox);
    baseLayout->addRow("Расстояние:", m_dashSpaceSpinBox);

    mainLayout->addWidget(baseGroup);

    // 2. Группа пользовательских стилей
    auto* customGroup = new QGroupBox("Конструктор стилей");
    auto* customLayout = new QVBoxLayout(customGroup);
    customLayout->setContentsMargins(15, 25, 15, 15);

    m_stylesListWidget = new QListWidget();
    m_stylesListWidget->setFixedHeight(100);

    //Заполняем список
    auto styles = LineStyleManager::instance().getCustomStyles();
    for (auto it = styles.begin(); it != styles.end(); ++it) {
        QListWidgetItem* item = new QListWidgetItem(it.value().name);
        item->setData(Qt::UserRole, it.key());
        m_stylesListWidget->addItem(item);
    }

    // Поле имени
    m_styleNameEdit = new QLineEdit();
    m_styleNameEdit->setPlaceholderText("Название нового стиля");

    // Кнопки конструктора
    auto* builderLayout = new QHBoxLayout();
    auto* addDashBtn = new QPushButton("Штрих");
    auto* addSpaceBtn = new QPushButton("Пробел");
    auto* addDotBtn = new QPushButton("Точка");
    auto* clearBtn = new QPushButton("Очистить");

    // Подсказки, что именно добавляется
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

    // Превью паттерна
    m_patternPreviewLabel = new QLabel("Шаблон: (пусто)");
    m_patternPreviewLabel->setStyleSheet("color: #00ff7f; font-weight: bold;");

    // Кнопки управления стилем
    auto* actionLayout = new QHBoxLayout();
    auto* addStyleBtn = new QPushButton("Сохранить стиль");
    auto* delStyleBtn = new QPushButton("Удалить выбранный");

    connect(addStyleBtn, &QPushButton::clicked, this, &SettingsWindow::onAddStyleClicked);
    connect(delStyleBtn, &QPushButton::clicked, this, &SettingsWindow::onDeleteStyleClicked);

    actionLayout->addWidget(addStyleBtn);
    actionLayout->addWidget(delStyleBtn);

    customLayout->addWidget(m_stylesListWidget);
    customLayout->addWidget(m_styleNameEdit);
    customLayout->addLayout(builderLayout);
    customLayout->addWidget(m_patternPreviewLabel);
    customLayout->addLayout(actionLayout);

    mainLayout->addWidget(customGroup);

    return widget;
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
        // Добавим пробел автоматически для симметрии
        m_currentPattern.append(5.0);
    }

    int newId = LineStyleManager::instance().generateNewId();
    LineStyleManager::instance().addCustomStyle(newId, name, m_currentPattern);

    //Обновление UI
    QListWidgetItem* item = new QListWidgetItem(name);
    item->setData(Qt::UserRole, newId);
    m_stylesListWidget->addItem(item);

    m_styleNameEdit->clear();
    onClearPattern(); // Сброс конструктора
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
}

void SettingsWindow::applySettings()
{
    //получение новых значений из UI
    QString selectedTheme = m_themeComboBox->currentData().toString();
    int newGridStep = m_gridStepSpinBox->value();
    double newZoomStep = m_zoomStepSpinBox->value();
    AngleUnit newAngleUnit = static_cast<AngleUnit>(m_angleUnitComboBox->currentData().toInt());
    double newThickness = m_lineThicknessSpinBox->value();
    double newDashLength = m_dashLengthSpinBox->value();
    double newDashSpace = m_dashSpaceSpinBox->value();

    //отправка значений в SettingsManager
    SettingsManager::instance().setThemeName(selectedTheme);
    SettingsManager::instance().setGridStep(newGridStep);
    SettingsManager::instance().setZoomStep(newZoomStep);
    SettingsManager::instance().setAngleUnit(newAngleUnit);
    SettingsManager::instance().setBaseLineThickness(newThickness);
    SettingsManager::instance().setDashLength(newDashLength);
    SettingsManager::instance().setDashSpace(newDashSpace);

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
