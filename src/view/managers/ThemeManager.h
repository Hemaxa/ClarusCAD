//ThemeManager - класс, отвечающий за внешнее оформление приложения

#pragma once

#include <QObject>
#include <QColor>
#include <QPixmap>
#include <QIcon>
#include <QMap>

class QSvgRenderer;

class ThemeManager : public QObject
{
    Q_OBJECT

public:
    //static означает, что метод можно вызвать без создания экземпляра класса (1 экземпляр)
    static ThemeManager& instance();

    //метод применения новой темы
    void applyTheme(const QString& themeName);

    //перегруженный метод для перекрашивания svg-файлов
    //1) для простых файлов с одним цветом (возвращает QIcon)
    static QIcon colorizeSvg(const QString& path, const QColor& color);

    //2) для сложных файлов с несколькими цветами (возвращает QPixmap)
    static QPixmap colorizeSvg(const QString& path, const QMap<QString, QColor>& colorMap);

    //геттеры цвета иконок и цветов из мапы цветов
    QColor getIconColor() const;
    QColor getColor(const QString& key) const;

private:
    //конструктор
    explicit ThemeManager(QObject *parent = nullptr);

    //запрет копирования и создания копий класса
    ThemeManager(const ThemeManager&) = delete;
    ThemeManager& operator=(const ThemeManager&) = delete;

    //метод работы с темой приложения
    void loadThemeFromFile(const QString& themeName);

    //метод получения цветов из файла темы
    void parseThemeColors(const QString& styleSheet);

    //метод замены svg
    static QString readAndReplaceSvg(const QString& path, const QMap<QString, QColor>& colorMap);

    //поля класса
    QColor m_iconColor; //текущий цвет иконок
    QMap<QString, QColor> m_themeColors; //все цвета из файла темы
};
