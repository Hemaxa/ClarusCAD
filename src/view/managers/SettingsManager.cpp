#include "SettingsManager.h"

SettingsManager& SettingsManager::instance()
{
    //конструктор будет вызван только один раз (переменная manager будет только одна)
    static SettingsManager manager;
    return manager;
}

SettingsManager::SettingsManager(QObject* parent) : QObject(parent), m_settings("MyCompany", "ClarusCAD") {}

void SettingsManager::loadSettings()
{
    //шаблон: "ключ настройки", "значение по умолчанию"
    m_currentThemeName = m_settings.value("theme/name", "ClarusCAD").toString();
    m_gridStep = m_settings.value("grid/step", 50).toInt();
    m_zoomStep = m_settings.value("zoom/step", 1.25).toDouble();
    m_angleUnit = static_cast<AngleUnit>(m_settings.value("angle/unit", static_cast<int>(AngleUnit::Degrees)).toInt());
    m_baseLineThickness = m_settings.value("line/base_thickness", 2.0).toDouble();
}

void SettingsManager::saveSettings()
{
    //шаблон: "ключ настройки", "значение, которое нужно сохранить"
    m_settings.setValue("theme/name", m_currentThemeName);
    m_settings.setValue("grid/step", m_gridStep);
    m_settings.setValue("zoom/step", m_zoomStep);
    m_settings.setValue("angle/unit", static_cast<int>(m_angleUnit));
    m_settings.setValue("line/base_thickness", m_baseLineThickness);
}

void SettingsManager::setThemeName(const QString& themeName)
{
    if (m_currentThemeName != themeName) {
        m_currentThemeName = themeName;
        emit themeNameChanged(m_currentThemeName);
    }
}

void SettingsManager::setGridStep(int step)
{
if (m_gridStep != step) {
        m_gridStep = step;
        emit gridStepChanged(m_gridStep);
    }
}

void SettingsManager::setZoomStep(double step)
{
    if (m_zoomStep != step) {
        m_zoomStep = step;
        emit zoomStepChanged(m_zoomStep);
    }
}

void SettingsManager::setAngleUnit(AngleUnit unit)
{
    if (m_angleUnit != unit) {
        m_angleUnit = unit;
        emit angleUnitChanged(m_angleUnit);
    }
}

void SettingsManager::setBaseLineThickness(double val)
{
    if (m_baseLineThickness != val) {
        m_baseLineThickness = val;
        emit baseLineThicknessChanged(val);
    }
}

QString SettingsManager::getThemeName() const { return m_currentThemeName; }
int SettingsManager::getGridStep() const { return m_gridStep; }
double SettingsManager::getZoomStep() const { return m_zoomStep; }
AngleUnit SettingsManager::getAngleUnit() const { return m_angleUnit; }
double SettingsManager::getBaseLineThickness() const { return m_baseLineThickness; }
