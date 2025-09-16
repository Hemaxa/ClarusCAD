#include "CommandParser.h"
#include <QRegularExpression>

CommandParser::CommandParser(QObject* parent) : QObject(parent) {}

ParsedCommand CommandParser::parse(const QString& commandString) const
{
    ParsedCommand result;
    // Регулярное выражение для поиска команд вида: "имя(число, число, ...)"
    // Оно ищет:
    // 1. ([a-zA-Z_]+) - имя команды (буквы и подчеркивание)
    // 2. \\s*\\( - открывающая скобка с пробелами до нее
    // 3. ([\\d\\s.,-]+) - аргументы (цифры, пробелы, точки, запятые, минусы)
    // 4. \\) - закрывающая скобка
    static QRegularExpression regex("([a-zA-Z_]+)\\s*\\(([\\d\\s.,-]+)\\)");
    QRegularExpressionMatch match = regex.match(commandString.trimmed());

    if (!match.hasMatch()) {
        return result; // Команда не соответствует шаблону
    }

    result.name = match.captured(1).toLower(); // Имя команды в нижнем регистре

    // Разделяем строку с аргументами на отдельные числа
    QStringList argStrings = match.captured(2).split(QRegularExpression("[\\s,]+"), Qt::SkipEmptyParts);

    for (const QString& argStr : argStrings) {
        bool ok;
        double value = argStr.toDouble(&ok);
        if (ok) {
            result.args.append(value);
        } else {
            // Если хотя бы один аргумент не является числом, команда недействительна
            return ParsedCommand();
        }
    }

    result.isValid = true;
    return result;
}
