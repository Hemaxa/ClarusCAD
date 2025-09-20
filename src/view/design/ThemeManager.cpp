#include "ThemeManager.h"

#include <QApplication>
#include <QFile>
#include <QTextStream>
#include <QRegularExpression>
#include <QDebug>

ThemeManager& ThemeManager::instance()
{
    static ThemeManager manager;
    return manager;
}

ThemeManager::ThemeManager(QObject *parent) : QObject(parent), m_settings("MyCompany", "ClarusCAD")
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

void ThemeManager::reloadTheme()
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

    //парсинг цветов
    parseThemeColors(styleSheet);

    //устанавливание цвета иконок из переменной
    m_iconColor = getColor("iconColor");

    qApp->setStyleSheet(styleSheet);
}

void ThemeManager::parseThemeColors(const QString& styleSheet)
{
    m_themeColors.clear();
    // Новое, более надежное регулярное выражение.
    // Оно ищет строку вида "@имя: значение;"
    QRegularExpression regex("@(\\w+):\\s*([^;]+);");

    auto it = regex.globalMatch(styleSheet);
    while (it.hasNext()) {
        QRegularExpressionMatch match = it.next();
        QString key = match.captured(1);

        // Получаем значение и очищаем его от возможных вложенных комментариев
        QString value = match.captured(2).trimmed();
        value = value.section("/*", 0, 0).trimmed(); // Убираем все, что после "/*"

        if (!value.isEmpty()) {
            m_themeColors.insert(key, QColor(value));
        }
    }
}

QIcon ThemeManager::colorizeSvgIcon(const QString& path, const QColor& color)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return QIcon();
    }

    QString svgData = QTextStream(&file).readAll();
    file.close();

    svgData.replace("currentColor", color.name(QColor::HexRgb));

    QByteArray svgBytes = svgData.toUtf8();
    return QIcon(QPixmap::fromImage(QImage::fromData(svgBytes)));
}

QColor ThemeManager::getIconColor() const { return m_iconColor; }
QColor ThemeManager::getColor(const QString& key) const { return m_themeColors.value(key, Qt::black); }
QString ThemeManager::getThemeName() const { return m_currentThemeName; }
