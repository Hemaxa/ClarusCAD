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
    /**
     * @brief Конструктор панели консоли.
     * @param title Заголовок панели.
     * @param parent Родительский виджет.
     */
    explicit ConsolePanelWidget(const QString& title, QWidget* parent = nullptr);

signals:
    /**
     * @brief Сигнал отправки команды.
     * Инициируется нажатием Enter в поле ввода.
     * @param command Распаршенная команда.
     */
    void commandParsed(const ParsedCommand& command);

private slots:
    /**
     * @brief Слот обработки нажатия Enter в поле ввода.
     */
    void onReturnPressed();

private:
    QLineEdit* m_commandInput; ///< Поле ввода текста команды
    CommandParser* m_commandParser; ///< Парсер команд
};
