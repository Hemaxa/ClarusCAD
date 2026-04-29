#include "CommandParser.h"
#include "SettingsManager.h"
#include "LinearDimensionCreationTool.h"
#include "ThemeManager.h"

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
    result.color = ThemeManager::instance().getColor("drawingColor");
    QString trimmedCommand = commandString.trimmed();

    //обработка пустой строки
    if (trimmedCommand.isEmpty()) {
        result.isValid = true;
        return result;
    }

    //разбор на имя команды и часть с параметрами
    static QRegularExpression regex("([a-zA-Z_]+)\\s*\\((.*)\\)");
    QRegularExpressionMatch match = regex.match(trimmedCommand);

    if (!match.hasMatch()) {
        result.errorDescription = "Ожидается формат: команда(...)";
        return result;
    }

    result.name = match.captured(1).toLower();
    QString argsPart = match.captured(2);

    if (result.name == "tool_linear_dimension") {
        // Как запрошено пользователем. Но мы уже обрабатываем это через сигналы-слоты MainWindow,
        // поэтому это просто заглушка для возврата, если парсинг когда-то расширится до инструментов.
        result.isValid = true;
        return result;
    }

    if (!m_commandInfos.contains(result.name)) {
        result.errorDescription = "Неизвестная команда: " + result.name;
        return result;
    }

    if (argsPart.trimmed().endsWith(','))
    {
        result.errorDescription = "Лишняя запятая в конце.";
        return result;
    }

    QStringList argStrings = argsPart.split(QRegularExpression("[\\s,]+"), Qt::SkipEmptyParts);

    if (!argStrings.isEmpty() && argStrings.last().startsWith('#')) {
        QColor color(argStrings.last());
        if (color.isValid()) {
            result.color = color;
            argStrings.removeLast();
        } else {
            result.errorDescription = "Неверный формат цвета.";
            return result;
        }
    }

    const CommandInfo& info = m_commandInfos[result.name];
    if (argStrings.size() != info.argCount) {
        result.errorDescription = QString("Ожидается %1 арг., получено %2.").arg(info.argCount).arg(argStrings.size());
        return result;
    }

    for (const QString& argStr : argStrings) {
        bool ok;
        double value = argStr.toDouble(&ok);
        if (ok) {
            result.args.append(value);
        } else {
            result.errorDescription = "Аргумент \"" + argStr + "\" не является числом.";
            return result;
        }
    }

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
