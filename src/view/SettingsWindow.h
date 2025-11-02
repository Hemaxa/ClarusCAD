//SettingsWindow - класс окна настроек приложения

#pragma once

#include "EnumManager.h"

#include <QDialog>

class QComboBox;
class QSpinBox;
class QDoubleSpinBox;

class SettingsWindow : public QDialog
{
    Q_OBJECT

public:
    //конструктор
    explicit SettingsWindow(QWidget* parent = nullptr);

    //методы установки текущих значений при открытии
    void setCurrentTheme(const QString& themeName);
    void setGridStep(int step);
    void setZoomStep(double step);
    void setAngleUnit(AngleUnit unit);

    //методы получения выбранных значений
    QString getCurrentTheme() const;
    int getGridStep() const;
    double getZoomStep() const;
    AngleUnit getAngleUnit() const;

private slots:
    //слот применения настроек
    void applySettings();

private:
    //метод заполнения списка тем
    void populateThemeComboBox();

    //элементы интерфейса
    QComboBox* m_themeComboBox;
    QSpinBox* m_gridStepSpinBox;
    QDoubleSpinBox* m_zoomStepSpinBox;
    QComboBox* m_angleUnitComboBox;
};
