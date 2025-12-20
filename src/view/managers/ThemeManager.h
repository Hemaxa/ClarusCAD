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
    /**
     * @brief Получить доступ к синглтону.
     */
    static ThemeManager& instance();

    /**
     * @brief Применить тему оформления.
     * @param themeName Имя темы (например, "Dark", "Light").
     */
    void applyTheme(const QString& themeName);

    /**
     * @brief Перекрасить SVG иконку в один цвет.
     * @param path Путь к файлу.
     * @param color Цвет.
     * @return Перекрашенная иконка (QIcon).
     */
    static QIcon colorizeSvg(const QString& path, const QColor& color);

    /**
     * @brief Перекрасить SVG иконку с заменой цветов по карте.
     * @param path Путь к файлу.
     * @param colorMap Карта замены цветов (ключ -> цвет).
     * @return Перекрашенное изображение (QPixmap).
     */
    static QPixmap colorizeSvg(const QString& path, const QMap<QString, QColor>& colorMap);

    // Геттеры цвета иконок и цветов из мапы цветов
    QColor getIconColor() const;
    QColor getColor(const QString& key) const;

signals:
    /**
     * @brief Сигнал завершения применения темы.
     */
    void themeApplied();

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
