#include "ConsolePanelWidget.h"
#include "CommandParser.h"

#include <QLineEdit>
#include <QVBoxLayout>
#include <QDebug>

ConsolePanelWidget::ConsolePanelWidget(const QString& title, QWidget* parent) : BasePanelWidget(title, parent)
{
    //создание парсера консольных команд
    m_commandParser = new CommandParser(this);

    //создание QLineEdit для консоли
    m_commandInput = new QLineEdit();

    //создание подсказки
    m_commandInput->setPlaceholderText("Введите команду...");

    auto* layout = new QVBoxLayout(canvas()); //вертикальный шаблон компоновки
    layout->setAlignment(Qt::AlignBottom); //выравнивание по нижнему краю

    layout->setContentsMargins(5, 5, 5, 5); //добавление отступов
    layout->addWidget(m_commandInput); //добавление содержимого панели в canvas

    //сигнал о нажатии Enter
    connect(m_commandInput, &QLineEdit::returnPressed, this, &ConsolePanelWidget::onReturnPressed);
}

void ConsolePanelWidget::onReturnPressed()
{
    //сохранение команды
    QString commandStr = m_commandInput->text();
    //если команда не пустая, то ее необходимо обработать
    if (!commandStr.isEmpty()) {
        ParsedCommand command = m_commandParser->parse(commandStr);

        if (command.isValid) {
            emit commandParsed(command);

        }
        else {
            //вывод сообщения об ошибке в отладочную консоль
            qDebug() << "Ошибка парсинга:" << command.errorDescription;
        }

        //очистка строки ввода
        m_commandInput->clear();
    }
}
