#include "SettingsDialog.h"

#include <QComboBox>
#include <QSpinBox>
#include <QFormLayout>
#include <QDialogButtonBox>
#include <QVBoxLayout>

SettingsDialog::SettingsDialog(QWidget* parent) : QDialog(parent)
{
    setWindowTitle("Настройки приложения");

    // Создание виджетов
    m_themeComboBox = new QComboBox();
    m_gridStepSpinBox = new QSpinBox();

    // Настройка виджетов
    populateThemeComboBox();
    m_gridStepSpinBox->setRange(10, 200); // Диапазон шага сетки
    m_gridStepSpinBox->setSingleStep(10);
    m_gridStepSpinBox->setSuffix(" px");  // Добавляем единицы измерения

    // Компоновка
    auto* formLayout = new QFormLayout();
    formLayout->addRow("Тема оформления:", m_themeComboBox);
    formLayout->addRow("Шаг сетки:", m_gridStepSpinBox);

    // Стандартные кнопки OK и Отмена
    auto* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->addLayout(formLayout);
    mainLayout->addWidget(buttonBox);

    setMinimumWidth(300);
}

void SettingsDialog::populateThemeComboBox()
{
    // Добавляем темы. Второй аргумент (userData) - это системное имя файла темы.
    m_themeComboBox->addItem("Тема ClarusCAD", "ClarusCAD");
    m_themeComboBox->addItem("Темная тема", "Dark");
    m_themeComboBox->addItem("Светлая тема", "Light");
}

QString SettingsDialog::selectedThemeName() const
{
    // Возвращаем системное имя темы из данных элемента
    return m_themeComboBox->currentData().toString();
}

int SettingsDialog::gridStep() const
{
    return m_gridStepSpinBox->value();
}

void SettingsDialog::setCurrentTheme(const QString& themeName)
{
    // Находим в списке тему с нужным системным именем
    int index = m_themeComboBox->findData(themeName);
    if (index != -1) {
        m_themeComboBox->setCurrentIndex(index);
    }
}

void SettingsDialog::setGridStep(int step)
{
    m_gridStepSpinBox->setValue(step);
}
