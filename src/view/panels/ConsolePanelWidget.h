//ConsolePanelWidget - панель консольного ввода

#pragma once

#include "BasePanelWidget.h"

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
    void commandEntered(const QString& command);

private slots:
    //слот обработки нажатия Enter
    void onReturnPressed();

private:
    QLineEdit* m_commandInput; //указатель на поле текста
};
