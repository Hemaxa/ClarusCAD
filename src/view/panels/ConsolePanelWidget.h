//ConsolePanelWidget - панель консольного ввода

#pragma once

#include "BasePanelWidget.h"
#include "CommandParser.h"

struct ParsedCommand;

class QLineEdit;

//наслдедуется от базового класса BasePanelWidget
class ConsolePanelWidget : public BasePanelWidget
{
    Q_OBJECT

public:
    //конструктор
    explicit ConsolePanelWidget(const QString& title, QWidget* parent = nullptr);

signals:
    //сигнал отправки команды нажатием Enter
    void commandParsed(const ParsedCommand& command);

private slots:
    //слот обработки нажатия Enter
    void onReturnPressed();

private:
    QLineEdit* m_commandInput; //указатель на поле текста
    CommandParser* m_commandParser; //указатель на парсер команд
};
