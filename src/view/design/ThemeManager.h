//ThemeManager - класс, отвечающий за внешнее оформление приложения

#pragma once

#include <QObject>
#include <QColor>
#include <QSettings>
#include <QIcon>
#include <QMap>

class ThemeManager : public QObject
{
    Q_OBJECT

public:
    //static означает, что метод можно вызвать без создания экземпляра класса
    static ThemeManager& instance();

    //метод применения новой темы
    void applyTheme(const QString& themeName);

    //метод загрузки последней темы
    void reloadTheme();

    //метод перекрашивания иконок
    static QIcon colorizeSvgIcon(const QString& path, const QColor& color);

    //геттеры цвета иконок, цвета из мапы цветов и текущей темы
    QColor getIconColor() const;
    QColor getColor(const QString& key) const;
    QString getThemeName() const;

private:
    //конструктор
    explicit ThemeManager(QObject *parent = nullptr);

    //запрет копирования и создания копий класса
    ThemeManager(const ThemeManager&) = delete;
    ThemeManager& operator=(const ThemeManager&) = delete;

    //методы работы с темами приложения
    void loadThemeFromFile(const QString& themeName);
    void saveSettings();
    void loadSettings();

    //метод получения цветов из файла темы
    void parseThemeColors(const QString& styleSheet);

    //поля класса
    QSettings m_settings;
    QString m_currentThemeName;
    QColor m_iconColor;
    QMap<QString, QColor> m_themeColors;
};
