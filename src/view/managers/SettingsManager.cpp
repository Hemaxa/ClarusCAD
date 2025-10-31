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
    m_angleUnit = static_cast<AngleUnit>(m_settings.value("angle/unit", static_cast<int>(AngleUnit::Degrees)).toInt());
}

void SettingsManager::saveSettings()
{
    //шаблон: "ключ настройки", "значение, которое нужно сохранить"
    m_settings.setValue("theme/name", m_currentThemeName);
    m_settings.setValue("grid/step", m_gridStep);
    m_settings.setValue("angle/unit", static_cast<int>(m_angleUnit));
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

void SettingsManager::setAngleUnit(AngleUnit unit)
{
    if (m_angleUnit != unit) {
        m_angleUnit = unit;
        emit angleUnitChanged(m_angleUnit);
    }
}

QString SettingsManager::getThemeName() const { return m_currentThemeName; }
int SettingsManager::getGridStep() const { return m_gridStep; }
AngleUnit SettingsManager::getAngleUnit() const { return m_angleUnit; }
