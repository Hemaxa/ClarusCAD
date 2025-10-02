//CommandParser - парсер для командной строки

#pragma once

#include <QObject>
#include <QString>
#include <QList>
#include <QColor>

//структура для хранения разобранной команды
struct ParsedCommand
{
    bool isValid = false; //флаг удачной обработки команды
    QString name; //имя команды
    QList<double> args; //список числовых аргументов команды
    QColor color; //заданный цвет объекта
};

class CommandParser : public QObject
{
    Q_OBJECT

public:
    //конструктор
    explicit CommandParser(QObject* parent = nullptr);

    //главный метод парсинга строки
    ParsedCommand parse(const QString& commandString) const;
};
