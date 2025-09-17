//ThemeManager - класс, отвечающий за внешнее представление приложения

#pragma once

#include <QObject>
#include <QColor>
#include <QSettings>
#include <QIcon>

class ThemeManager : public QObject
{
    Q_OBJECT

public:
    //static означает, что метод можно вызвать без создания экземпляра класса
    static ThemeManager& instance();

    //метод применения новой темы
    void applyTheme(const QString& themeName);

    //метод загрузки последней темы
    void reloadCurrentTheme();

    //метод перекрашивания иконок
    static QIcon colorizeSvgIcon(const QString& path, const QColor& color);

    //геттеры цвета иконок и текущей темы
    QColor getIconColor() const;
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

    //поля класса
    QSettings m_settings;
    QString m_currentThemeName;
    QColor m_iconColor;
};
