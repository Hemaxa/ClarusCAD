//SettingsManager - класс, отвечающий за сохранение и загрузку настроек приложения

#pragma once

#include "EnumManager.h"

#include <QObject>
#include <QSettings> //класс Qt для работы с настройками (умеет сохранять настройки в файл или реестр)
#include <QString>

class SettingsManager : public QObject
{
    Q_OBJECT

public:
    //static означает, что метод можно вызвать без создания экземпляра класса (1 экземпляр)
    static SettingsManager& instance();

    //методы загрузки и сохранения всех настроек
    void loadSettings();
    void saveSettings();

    //геттеры и сеттеры для настроек
    //тема приложения
    void setThemeName(const QString& themeName);
    QString getThemeName() const;

    //шаг сетки
    void setGridStep(int step);
    int getGridStep() const;

    //шаг увеличения/уменьшения
    void setZoomStep(double step);
    double getZoomStep() const;

    //единицы измерения углов
    void setAngleUnit(AngleUnit unit);
    AngleUnit getAngleUnit() const;

signals:
    //сигналы об изменении настроек
    void themeNameChanged(const QString& themeName);
    void gridStepChanged(int step);
    void zoomStepChanged(double step);
    void angleUnitChanged(AngleUnit unit);

private:
    //конструктор
    explicit SettingsManager(QObject* parent = nullptr);

    //запрет копирования и создания копий класса
    SettingsManager(const SettingsManager&) = delete;
    SettingsManager& operator=(const SettingsManager&) = delete;

    //объект для чтения и записи настроек
    QSettings m_settings;

    //поля хранения текущих настроек
    QString m_currentThemeName;
    int m_gridStep;
    double m_zoomStep;
    AngleUnit m_angleUnit;
};
