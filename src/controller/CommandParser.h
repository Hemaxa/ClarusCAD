//CommandParser - парсер для командной строки

#pragma once

#include "EnumManager.h"

#include <QObject>
#include <QString>
#include <QMap>
#include <QList>
#include <QColor>

//структура для хранения информации о команде
struct CommandInfo
{
    QString syntaxCartesian; //подсказка для декартовой системы координат
    QString syntaxPolar; //подсказка для полярной системы координат
    int argCount; //количество аргументов
};

//структура для хранения разобранной команды
struct ParsedCommand
{
    bool isValid = false; //флаг удачной обработки команды
    QString name; //имя команды
    QList<double> args; //список числовых аргументов команды
    QColor color; //заданный цвет объекта
    QString errorDescription; //текстовое описание ошибки
};

class CommandParser : public QObject
{
    Q_OBJECT

public:
    //конструктор
    explicit CommandParser(QObject* parent = nullptr);

    //главный метод парсинга строки
    ParsedCommand parse(const QString& commandString) const;

    //геттер для подсказки
    QString getHint(const QString& commandName, CoordinateSystemType coordSystem) const;

private:
    //хранилище с информацией о всех известных командах
    QMap<QString, CommandInfo> m_commandInfos;
};
