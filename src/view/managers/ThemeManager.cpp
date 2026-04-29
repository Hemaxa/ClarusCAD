#include "ThemeManager.h"

#include <QApplication>
#include <QFile>
#include <QTextStream>
#include <QRegularExpression>
#include <QPainter>
#include <QDebug>

ThemeManager& ThemeManager::instance()
{
    //конструктор будет вызван только один раз (переменная manager будет только одна)
    static ThemeManager manager;
    return manager;
}

ThemeManager::ThemeManager(QObject *parent) : QObject(parent) {}

void ThemeManager::applyTheme(const QString& themeName)
{
    loadThemeFromFile(themeName);
}

void ThemeManager::loadThemeFromFile(const QString& themeName)
{
    //чтение файла выбранной темы
    QString filePath = QString(":/themes/themes/%1.qss").arg(themeName);
    QFile file(filePath);

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning("Не удалось открыть файл темы: %s", qUtf8Printable(filePath));
        return;
    }

    //запись всего содержимого файла в строку
    QString styleSheet = file.readAll();
    file.close();

    //парсинг цветов из строки
    parseThemeColors(styleSheet);

    //устанавливание цвета иконок из переменной
    m_iconColor = getColor("iconColor");

    //применение стиля ко всему приложению
    qApp->setStyleSheet(styleSheet);

    //отправка сигнала, что тема изменилась
    emit themeApplied();
}

//перегрузка 1: один цвет
QIcon ThemeManager::colorizeSvg(const QString& path, const QColor& color)
{
    //создается карта с одним элементом для замены "currentColor"
    QMap<QString, QColor> colorMap;
    colorMap.insert("currentColor", color);

    //вызов функции замены readAndReplaceSvg
    QString svgData = readAndReplaceSvg(path, colorMap);
    if (svgData.isEmpty()) {
        return QIcon();
    }

    //сохранение и возврат
    QByteArray svgBytes = svgData.toUtf8();
    QPixmap pixmap;
    pixmap.loadFromData(svgBytes, "SVG");
    return QIcon(pixmap);
}

//перегрузка 2: несколько цветов
QPixmap ThemeManager::colorizeSvg(const QString& path, const QMap<QString, QColor>& colorMap)
{
    //вызов функции замены readAndReplaceSvg
    QString svgData = readAndReplaceSvg(path, colorMap);
    if (svgData.isEmpty()) {
        return QPixmap();
    }

    //сохранение и возврат
    QByteArray svgBytes = svgData.toUtf8();
    QPixmap pixmap;
    pixmap.loadFromData(svgBytes, "SVG");
    return pixmap;
}

void ThemeManager::parseThemeColors(const QString& styleSheet)
{
    //очистка текущих цветов
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
    //проверка окрытия файла
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Не удалось открыть SVG-файл:" << path;
        return QString();
    }

    //запись всего svg-файла в строку
    QString svgData = QTextStream(&file).readAll();
    file.close();

    //проход по всем парам "плейсхолдер-цвет" и замена их
    for (auto it = colorMap.constBegin(); it != colorMap.constEnd(); ++it) {
        svgData.replace(it.key(), it.value().name(QColor::HexRgb));
    }

    if (colorMap.contains("currentColor")) {
        const QString themedColor = colorMap.value("currentColor").name(QColor::HexRgb);
        const QStringList legacyThemeColors = {
            "#00ff7f",
            "#00FF7F",
            "#00ff80",
            "#00FF80",
            "rgb(0,255,127)",
            "rgb(0, 255, 127)"
        };

        for (const QString& legacyColor : legacyThemeColors) {
            svgData.replace(legacyColor, themedColor);
        }
    }

    return svgData;
}

QColor ThemeManager::getIconColor() const { return m_iconColor; }
QColor ThemeManager::getColor(const QString& key) const { return m_themeColors.value(key, Qt::white); }
