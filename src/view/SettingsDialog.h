#pragma once

#include <QDialog>

// Прямые объявления для уменьшения времени компиляции
class QComboBox;
class QSpinBox;

class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsDialog(QWidget* parent = nullptr);

    // Методы для получения выбранных значений
    QString selectedThemeName() const;
    int gridStep() const;

    // Методы для установки текущих значений при открытии
    void setCurrentTheme(const QString& themeName);
    void setGridStep(int step);

private:
    void populateThemeComboBox();

    QComboBox* m_themeComboBox;
    QSpinBox* m_gridStepSpinBox;
};
