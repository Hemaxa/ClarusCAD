#include "ThemeManager.h"

#include <QApplication>
#include <QFile>
#include <QTextStream>
#include <QRegularExpression>
#include <QPainter>
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

QIcon ThemeManager::colorizeSvg(const QString& path, const QColor& color)
{
    // Создаем карту с одним элементом для замены "currentColor"
    QMap<QString, QColor> colorMap;
    colorMap.insert("currentColor", color);

    // Вызываем нашу общую функцию
    QString svgData = readAndReplaceSvg(path, colorMap);
    if (svgData.isEmpty()) {
        return QIcon();
    }

    QByteArray svgBytes = svgData.toUtf8();
    QPixmap pixmap;
    pixmap.loadFromData(svgBytes, "SVG");
    return QIcon(pixmap);
}

// Перегрузка #2: для подсказок с несколькими цветами
QPixmap ThemeManager::colorizeSvg(const QString& path, const QMap<QString, QColor>& colorMap)
{
    // Вызываем нашу общую функцию
    QString svgData = readAndReplaceSvg(path, colorMap);
    if (svgData.isEmpty()) {
        return QPixmap();
    }

    QByteArray svgBytes = svgData.toUtf8();
    QPixmap pixmap;
    pixmap.loadFromData(svgBytes, "SVG");
    return pixmap;
}

void ThemeManager::parseThemeColors(const QString& styleSheet)
{
    m_themeColors.clear();

    //парсинг переменных цветов из .qss файла
    QRegularExpression regex("@(\\w+):\\s*([^;]+);");

    auto it = regex.globalMatch(styleSheet);
    while (it.hasNext()) {
        QRegularExpressionMatch match = it.next();
        QString key = match.captured(1);

        //получение значения и очистка от возможных комментариев
        QString value = match.captured(2).trimmed();
        value = value.section("/*", 0, 0).trimmed(); //убирает все, что после "/*"

        if (!value.isEmpty()) {
            m_themeColors.insert(key, QColor(value));
        }
    }
}

QString ThemeManager::readAndReplaceSvg(const QString& path, const QMap<QString, QColor>& colorMap)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Could not open SVG file:" << path;
        return QString(); // Возвращаем пустую строку в случае ошибки
    }

    QString svgData = QTextStream(&file).readAll();
    file.close();

    // Проходим по всем парам "плейсхолдер-цвет" и заменяем их
    for (auto it = colorMap.constBegin(); it != colorMap.constEnd(); ++it) {
        svgData.replace(it.key(), it.value().name(QColor::HexRgb));
    }

    return svgData;
}

QColor ThemeManager::getIconColor() const { return m_iconColor; }
QColor ThemeManager::getColor(const QString& key) const { return m_themeColors.value(key, Qt::white); }
QString ThemeManager::getThemeName() const { return m_currentThemeName; }
