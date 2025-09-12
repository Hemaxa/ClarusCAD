#include "SettingsDialog.h"

#include <QVBoxLayout>
#include <QFormLayout>
#include <QComboBox>
#include <QDialogButtonBox>

SettingsDialog::SettingsDialog(QWidget* parent) : QDialog(parent)
{
    setWindowTitle("Настройки");

    auto* mainLayout = new QVBoxLayout(this);
    auto* formLayout = new QFormLayout();

    // Создание выпадающего списка для выбора темы
    m_themeComboBox = new QComboBox();
    m_themeComboBox->addItem("Стандартная", ""); // Путь пустой для темы по умолчанию
    m_themeComboBox->addItem("Темно-зеленая", ":/themes/DarkAcidGreen.qss");

    formLayout->addRow("Тема:", m_themeComboBox);
    mainLayout->addLayout(formLayout);

    // Создание кнопок OK и Cancel
    auto* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &SettingsDialog::applySettings);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    mainLayout->addWidget(buttonBox);
}

void SettingsDialog::applySettings()
{
    // Получаем путь к теме из данных выбранного элемента
    QString themePath = m_themeComboBox->currentData().toString();
    emit themeChanged(themePath);
    accept(); // Закрываем диалог с успехом
}
