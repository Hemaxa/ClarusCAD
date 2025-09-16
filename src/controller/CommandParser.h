#pragma once

#include <QObject>
#include <QString>
#include <QList>

// Структура для хранения разобранной команды
struct ParsedCommand
{
    bool isValid = false;
    QString name;
    QList<double> args;
};

class CommandParser : public QObject
{
    Q_OBJECT

public:
    explicit CommandParser(QObject* parent = nullptr);

    // Главный метод, который разбирает строку
    ParsedCommand parse(const QString& commandString) const;
};
