//CommandParser - парсер для командной строки

#pragma once

#include "EnumManager.h"

#include <QObject>
#include <QString>
#include <QMap>
#include <QList>
#include <QColor>

//структура для хранения информации о команде
/**
 * @brief Структура для хранения информации о синтаксисе команды.
 */
struct CommandInfo
{
    QString syntaxCartesian; ///< Подсказка для декартовой системы
    QString syntaxPolar;     ///< Подсказка для полярной системы
    int argCount;            ///< Ожидаемое количество аргументов
};

//структура для хранения разобранной команды
/**
 * @brief Результат парсинга команды.
 */
struct ParsedCommand
{
    bool isValid = false;     ///< Успешен ли парсинг
    QString name;             ///< Имя команды
    QList<double> args;       ///< Числовые аргументы
    QColor color;             ///< Цвет для создаваемого объекта (если указан)
    QString errorDescription; ///< Текст ошибки
};

class CommandParser : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief Конструктор парсера.
     */
    explicit CommandParser(QObject* parent = nullptr);

    /**
     * @brief Распарсить строку команды.
     * @param commandString Строка ввода от пользователя.
     * @return Структура ParsedCommand с результатом.
     */
    ParsedCommand parse(const QString& commandString) const;

    /**
     * @brief Получить подсказку по синтаксису команды.
     * @param commandName Имя команды.
     * @param coordSystem Текущая система координат.
     * @return Строка подсказки.
     */
    QString getHint(const QString& commandName, CoordinateSystemType coordSystem) const;

private:
    QMap<QString, CommandInfo> m_commandInfos; ///< Реестр команд
};
