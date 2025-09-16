#include "ConsolePanelWidget.h"
#include <QLineEdit>
#include <QVBoxLayout>

ConsolePanelWidget::ConsolePanelWidget(const QString& title, QWidget* parent)
    : BasePanelWidget(title, parent)
{
    m_commandInput = new QLineEdit();
    m_commandInput->setPlaceholderText("Введите команду...");

    auto* layout = new QVBoxLayout(canvas());
    layout->setContentsMargins(5, 5, 5, 5);
    layout->addWidget(m_commandInput);

    // Соединяем сигнал нажатия Enter со слотом
    connect(m_commandInput, &QLineEdit::returnPressed, this, &ConsolePanelWidget::onReturnPressed);
}

void ConsolePanelWidget::onReturnPressed()
{
    QString command = m_commandInput->text();
    if (!command.isEmpty()) {
        emit commandEntered(command);
        m_commandInput->clear();
    }
}
