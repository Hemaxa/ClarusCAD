#pragma once

#include <QObject>
#include <QColor>
#include <QSettings>
#include <QIcon>

class ThemeManager : public QObject
{
    Q_OBJECT

public:
    static ThemeManager& instance();

    void applyTheme(const QString& themeName);
    void reloadCurrentTheme();

    QColor getIconColor() const;
    QString currentThemeName() const;

    static QIcon colorizeSvgIcon(const QString& path, const QColor& color);

private:
    explicit ThemeManager(QObject *parent = nullptr);
    ThemeManager(const ThemeManager&) = delete;
    ThemeManager& operator=(const ThemeManager&) = delete;

    void loadThemeFromFile(const QString& themeName);
    void saveSettings();
    void loadSettings();

    QSettings m_settings;
    QString m_currentThemeName;
    QColor m_iconColor;
};
