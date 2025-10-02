#include "ConsolePanelWidget.h"

#include <QLineEdit>
#include <QVBoxLayout>

ConsolePanelWidget::ConsolePanelWidget(const QString& title, QWidget* parent) : BasePanelWidget(title, parent)
{
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
    QString command = m_commandInput->text();
    //если команда не пустая, то ее отправка
    if (!command.isEmpty()) {
        emit commandEntered(command);

        //очистка строки ввода
        m_commandInput->clear();
    }
}
