//SettingsManager - класс, отвечающий за сохранение и загрузку настроек приложения

#pragma once

#include "EnumManager.h"

#include <QObject>
#include <QSettings> //класс Qt для работы с настроками (умеет сохранять настройки в файл или реестр)
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

    //толщина линий
    void setBaseLineThickness(double val);
    double getBaseLineThickness() const;

    //длина штриха
    void setDashLength(double val);
    double getDashLength() const;

    //расстояние между штрихами
    void setDashSpace(double val);
    double getDashSpace() const;

    // --- Параметры волнистой линии ---
    void setWaveAmplitude(double val);
    double getWaveAmplitude() const;
    void setWavePeriod(double val);
    double getWavePeriod() const;

    // --- Параметры линии с изломами ---
    void setKinkAmplitude(double val);
    double getKinkAmplitude() const;
    void setKinkLength(double val);
    double getKinkLength() const;
    void setKinkStraight(double val);
    double getKinkStraight() const;

signals:
    //сигналы об изменении настроек
    void themeNameChanged(const QString& themeName);
    void gridStepChanged(int step);
    void zoomStepChanged(double step);
    void angleUnitChanged(AngleUnit unit);
    void baseLineThicknessChanged(double thickness);
    void dashLengthChanged(double length);
    void dashSpaceChanged(double space);
    void waveParamsChanged();
    void kinkParamsChanged();

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
    double m_baseLineThickness;
    double m_dashLength;
    double m_dashSpace;

    // Параметры волнистой линии
    double m_waveAmplitude;
    double m_wavePeriod;

    // Параметры линии с изломами
    double m_kinkAmplitude;
    double m_kinkLength;
    double m_kinkStraight;
};
