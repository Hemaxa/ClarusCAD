//SettingsWindow - класс окна настроек приложения

#pragma once

#include "EnumManager.h"

#include <QDialog>

class QComboBox;
class QSpinBox;

class SettingsWindow : public QDialog
{
    Q_OBJECT

public:
    //конструктор
    explicit SettingsWindow(QWidget* parent = nullptr);

    //методы получения выбранных значений
    QString getCurrentTheme() const;
    int getGridStep() const;
    AngleUnit getAngleUnit() const;

    //методы  установки текущих значений при открытии
    void setCurrentTheme(const QString& themeName);
    void setGridStep(int step);
    void setAngleUnit(AngleUnit unit);

private:
    //метод заполнения списка тем
    void populateThemeComboBox();

    //элементы интерфейса
    QComboBox* m_themeComboBox;
    QSpinBox* m_gridStepSpinBox;
    QComboBox* m_angleUnitComboBox;
};
