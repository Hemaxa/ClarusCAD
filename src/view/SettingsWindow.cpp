#include "SettingsWindow.h"

#include <QComboBox>
#include <QSpinBox>
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

    //настройка элементов интерфейса
    //настройка шага сетки
    populateThemeComboBox();
    m_gridStepSpinBox->setRange(10, 200);
    m_gridStepSpinBox->setSingleStep(10);
    m_gridStepSpinBox->setSuffix(" px");

    //настройка списка тем
    m_themeComboBox->setFixedHeight(30);

    //расположение элементов интерфейса
    auto* appearanceGroup = new QGroupBox("Оформление");
    auto* formLayout = new QFormLayout();
    formLayout->setSpacing(15);
    formLayout->addRow("Тема оформления:", m_themeComboBox);
    formLayout->addRow("Шаг сетки:", m_gridStepSpinBox);
    appearanceGroup->setLayout(formLayout);

    //кнопки "OK" и "Отмена"
    auto* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    //сборка всего диалогового окна вместе
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(15, 15, 15, 15);
    mainLayout->setSpacing(15);
    mainLayout->addWidget(appearanceGroup);
    mainLayout->addWidget(buttonBox);
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

QString SettingsWindow::getCurrentTheme() const { return m_themeComboBox->currentData().toString(); }
int SettingsWindow::getGridStep() const { return m_gridStepSpinBox->value(); }
