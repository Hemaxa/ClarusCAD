#include "ThemeManager.h"
#include <QApplication>
#include <QFile>
#include <QTextStream>
#include <QRegularExpression>
#include <QDebug> // Для отладки

ThemeManager& ThemeManager::instance()
{
    static ThemeManager manager;
    return manager;
}

ThemeManager::ThemeManager(QObject *parent)
    : QObject(parent), m_settings("MyCompany", "ClarusCAD")
{
    loadSettings();
}

void ThemeManager::loadSettings()
{
    m_currentThemeName = m_settings.value("theme/name", "Dark").toString();
}

void ThemeManager::saveSettings()
{
    m_settings.setValue("theme/name", m_currentThemeName);
}

void ThemeManager::applyTheme(const QString& themeName)
{
    loadThemeFromFile(themeName);
    saveSettings();
}

void ThemeManager::reloadCurrentTheme()
{
    loadThemeFromFile(m_currentThemeName);
}

void ThemeManager::loadThemeFromFile(const QString& themeName)
{
    m_currentThemeName = themeName;
    QString filePath = QString(":/themes/themes/%1.qss").arg(themeName);
    QFile file(filePath);

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning("Could not open theme file: %s", qUtf8Printable(filePath));
        return;
    }

    QString styleSheet = file.readAll();
    file.close();

    QRegularExpression regex("/\\*\\s*@iconColor:\\s*([#\\w\\(\\),\\s]+);\\s*\\*/");
    QRegularExpressionMatch match = regex.match(styleSheet);
    if (match.hasMatch()) {
        m_iconColor = QColor(match.captured(1).trimmed());
    } else {
        m_iconColor = Qt::black; // Fallback color
    }

    qApp->setStyleSheet(styleSheet);
}

QColor ThemeManager::getIconColor() const { return m_iconColor; }
QString ThemeManager::currentThemeName() const { return m_currentThemeName; }

QIcon ThemeManager::colorizeSvgIcon(const QString& path, const QColor& color)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return QIcon();
    }

    QString svgData = QTextStream(&file).readAll();
    file.close();

    svgData.replace("currentColor", color.name(QColor::HexRgb));
    svgData.replace("#000000", color.name(QColor::HexRgb));
    svgData.replace("black", color.name(QColor::HexRgb));

    QByteArray svgBytes = svgData.toUtf8();
    return QIcon(QPixmap::fromImage(QImage::fromData(svgBytes)));
}
