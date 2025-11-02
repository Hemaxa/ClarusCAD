#include "SettingsWindow.h"
#include "SettingsManager.h"

#include <QComboBox>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QDialogButtonBox>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QLabel>

SettingsWindow::SettingsWindow(QWidget* parent) : QDialog(parent)
{
    //настройки окна
    setWindowTitle("Настройки");
    setMinimumWidth(400);
    setMinimumHeight(400);

    //создание элементов интерфейса
    m_themeComboBox = new QComboBox();
    m_gridStepSpinBox = new QSpinBox();
    m_zoomStepSpinBox = new QDoubleSpinBox();
    m_angleUnitComboBox = new QComboBox();

    //настройка элементов интерфейса
    //настройка списка тем
    populateThemeComboBox();
    m_themeComboBox->setFixedHeight(30);

    //настройка шага сетки
    m_gridStepSpinBox->setRange(10, 100);
    m_gridStepSpinBox->setSingleStep(5);
    m_gridStepSpinBox->setSuffix(" px");
    m_gridStepSpinBox->setFixedHeight(30);

    //настройка шага увеличения/уменьшения
    m_zoomStepSpinBox->setRange(1.10, 3.00);
    m_zoomStepSpinBox->setSingleStep(0.05);
    m_zoomStepSpinBox->setDecimals(2);
    m_zoomStepSpinBox->setSuffix("x");
    m_zoomStepSpinBox->setFixedHeight(30);

    //настройка единиц измерения углов
    m_angleUnitComboBox->addItem("Градусы", static_cast<int>(AngleUnit::Degrees));
    m_angleUnitComboBox->addItem("Радианы", static_cast<int>(AngleUnit::Radians));
    m_angleUnitComboBox->setFixedHeight(30);

    //установка текущиех значений для полей
    setCurrentTheme(SettingsManager::instance().getThemeName());
    setGridStep(SettingsManager::instance().getGridStep());
    setZoomStep(SettingsManager::instance().getZoomStep());
    setAngleUnit(SettingsManager::instance().getAngleUnit());

    //расположение элементов интерфейса
    auto* appearanceGroup = new QGroupBox("Оформление");
    auto* formLayout = new QFormLayout();
    formLayout->setSpacing(15);

    formLayout->addRow("Тема оформления:", m_themeComboBox);
    formLayout->addRow("Шаг сетки:", m_gridStepSpinBox);
    formLayout->addRow("Шаг увеличения/уменьшения:", m_zoomStepSpinBox);
    formLayout->addRow("Единицы измерения углов:", m_angleUnitComboBox);

    appearanceGroup->setLayout(formLayout);

    //кнопки "OK" и "Отмена"
    auto* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &SettingsWindow::applySettings);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    //сборка всего диалогового окна вместе
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(15, 15, 15, 15);
    mainLayout->setSpacing(15);
    mainLayout->addWidget(appearanceGroup);
    mainLayout->addWidget(buttonBox);
}

void SettingsWindow::applySettings()
{
    //получение новых значений из UI
    QString selectedTheme = getCurrentTheme();
    int newGridStep = getGridStep();
    double newZoomStep = getZoomStep();
    AngleUnit newAngleUnit = getAngleUnit();

    //отправка значений в SettingsManager
    SettingsManager::instance().setThemeName(selectedTheme);
    SettingsManager::instance().setGridStep(newGridStep);
    SettingsManager::instance().setZoomStep(newZoomStep);
    SettingsManager::instance().setAngleUnit(newAngleUnit);

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

void SettingsWindow::setCurrentTheme(const QString& themeName) { int index = m_themeComboBox->findData(themeName); if (index != -1) { m_themeComboBox->setCurrentIndex(index); } }
void SettingsWindow::setGridStep(int step) { m_gridStepSpinBox->setValue(step); }
void SettingsWindow::setZoomStep(double step) { m_zoomStepSpinBox->setValue(step); }
void SettingsWindow::setAngleUnit(AngleUnit unit) { int index = m_angleUnitComboBox->findData(static_cast<int>(unit)); if (index != -1) { m_angleUnitComboBox->setCurrentIndex(index); } }

QString SettingsWindow::getCurrentTheme() const { return m_themeComboBox->currentData().toString(); }
int SettingsWindow::getGridStep() const { return m_gridStepSpinBox->value(); }
double SettingsWindow::getZoomStep() const { return m_zoomStepSpinBox->value(); }
AngleUnit SettingsWindow::getAngleUnit() const { return static_cast<AngleUnit>(m_angleUnitComboBox->currentData().toInt()); }
