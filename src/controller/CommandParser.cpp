#include "CommandParser.h"
#include "SettingsManager.h"

#include <QRegularExpression>

CommandParser::CommandParser(QObject* parent) : QObject(parent)
{
    //заполняется информация о поддерживаемых командах для обеих систем
    QString angleUnitHint = (SettingsManager::instance().getAngleUnit() == AngleUnit::Degrees) ? "°" : "rad";
    m_commandInfos["segment"] = {
        "segment(x1, y1, x2, y2, #цвет)",
        QString("segment(r1, a1%1, r2, a2%1, #цвет)").arg(angleUnitHint),
        4
    };
}
ParsedCommand CommandParser::parse(const QString& commandString) const
{
    ParsedCommand result;
    result.color = Qt::white; //цвет по умолчанию

    //1. ([a-zA-Z_]+)
    //имя команды (первая захватывающая группа)
    //ищет последовательность из одной или более (+) больших или маленьких букв латинского алфавита или нижнего подчеркивания ([a-zA-Z_])
    //2. \\s*
    //отступ
    //ищет ноль или более (*) пробельных символов (\\s)
    //3. \\(
    //открывающаяся скобка
    //ищет открывающуюся скобку (\ нужен для экранирования, т.к. скобка специальный символ)
    //4. (.+)
    //строка с аргументами (вторая захватывающая группа)
    //ищет любой символ (.) один или более раз (+)
    //5. \\)
    //закрывающаяся скобка
    //ищет закрывающуюся скобку

    static QRegularExpression regex("([a-zA-Z_]+)\\s*\\((.+)\\)");
    QRegularExpressionMatch match = regex.match(commandString.trimmed());

    //валидация не пройдена
    if (!match.hasMatch()) {
        return result;
    }

    //перевод команды в нижний регистр
    result.name = match.captured(1).toLower();

    //создание списка параметров
    QStringList argStrings = match.captured(2).split(QRegularExpression("[\\s,]+"), Qt::SkipEmptyParts);

    //проверка наличия параметра цвета
    if (!argStrings.isEmpty() && argStrings.last().startsWith('#')) {
        QColor color(argStrings.last());
        //если получилось перевести в цвет
        if (color.isValid()) {
            result.color = color; //сохранение цвета
            argStrings.removeLast(); //удаление цвета из списка аргументов
        }
    }

    //разбор списка параметров
    for (const QString& argStr : argStrings) {
        bool ok;
        double value = argStr.toDouble(&ok);
        if (ok) {
            result.args.append(value);
        }
        else {
            return ParsedCommand();
        }
    }

    //если все прошло хорошо, то
    result.isValid = true;
    return result;
}

QString CommandParser::getHint(const QString& commandName, CoordinateSystemType coordSystem) const
{
    if (m_commandInfos.contains(commandName)) {
        if (coordSystem == CoordinateSystemType::Polar) {
            return m_commandInfos[commandName].syntaxPolar;
        }
        return m_commandInfos[commandName].syntaxCartesian;
    }
    return "Неизвестная команда.";
}
