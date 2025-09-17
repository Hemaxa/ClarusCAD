//SettingsDialog - класс окна настроек приложения

#pragma once

#include <QDialog>

class QComboBox;
class QSpinBox;

class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    //конструктор
    explicit SettingsDialog(QWidget* parent = nullptr);

    //методы получения выбранных значений
    QString getCurrentTheme() const;
    int getGridStep() const;

    //методы  установки текущих значений при открытии
    void setCurrentTheme(const QString& themeName);
    void setGridStep(int step);

private:
    //метод заполнения списка тем
    void populateThemeComboBox();

    //элементы интерфейса
    QComboBox* m_themeComboBox;
    QSpinBox* m_gridStepSpinBox;
};
