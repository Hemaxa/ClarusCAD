#include "SettingsManager.h"
#include "LineStyleManager.h"

#include <QStringList>

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
    m_dashLength = m_settings.value("line/dash_length", 10.0).toDouble();
    m_dashSpace = m_settings.value("line/dash_space", 5.0).toDouble();

    //Загрузка пользовательских линий
    int size = m_settings.beginReadArray("custom_lines");
    for (int i = 0; i < size; ++i) {
        m_settings.setArrayIndex(i);
        int id = m_settings.value("id").toInt();
        QString name = m_settings.value("name").toString();

        // Паттерн сохраняем как строку "10,5,2,5"
        QString patternStr = m_settings.value("pattern").toString();
        QVector<qreal> pattern;
        if (!patternStr.isEmpty()) {
            QStringList parts = patternStr.split(',');
            for (const QString& p : parts) {
                pattern.append(p.toDouble());
            }
        }

        LineStyleManager::instance().addCustomStyle(id, name, pattern);
    }
    m_settings.endArray();
}

void SettingsManager::saveSettings()
{
    //шаблон: "ключ настройки", "значение, которое нужно сохранить"
    m_settings.setValue("theme/name", m_currentThemeName);
    m_settings.setValue("grid/step", m_gridStep);
    m_settings.setValue("zoom/step", m_zoomStep);
    m_settings.setValue("angle/unit", static_cast<int>(m_angleUnit));
    m_settings.setValue("line/base_thickness", m_baseLineThickness);
    m_settings.setValue("line/dash_length", m_dashLength);
    m_settings.setValue("line/dash_space", m_dashSpace);

    //Сохранение пользовательских линий
    auto customStyles = LineStyleManager::instance().getCustomStyles();
    m_settings.beginWriteArray("custom_lines");
    int i = 0;
    for (auto it = customStyles.begin(); it != customStyles.end(); ++it) {
        m_settings.setArrayIndex(i++);
        m_settings.setValue("id", it.key());
        m_settings.setValue("name", it.value().name);

        QStringList pList;
        for (qreal v : it.value().pattern) {
            pList << QString::number(v);
        }
        m_settings.setValue("pattern", pList.join(','));
    }
    m_settings.endArray();
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

void SettingsManager::setDashLength(double val)
{
    if (m_dashLength != val) {
        m_dashLength = val;
        emit dashLengthChanged(val);
    }
}

void SettingsManager::setDashSpace(double val)
{
    if (m_dashSpace != val) {
        m_dashSpace = val;
        emit dashSpaceChanged(val);
    }
}

QString SettingsManager::getThemeName() const { return m_currentThemeName; }
int SettingsManager::getGridStep() const { return m_gridStep; }
double SettingsManager::getZoomStep() const { return m_zoomStep; }
AngleUnit SettingsManager::getAngleUnit() const { return m_angleUnit; }
double SettingsManager::getBaseLineThickness() const { return m_baseLineThickness; }
double SettingsManager::getDashLength() const { return m_dashLength; }
double SettingsManager::getDashSpace() const { return m_dashSpace; }
