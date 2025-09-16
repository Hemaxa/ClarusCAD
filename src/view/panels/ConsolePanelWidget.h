#pragma once

#include "BasePanelWidget.h"

class QLineEdit;

class ConsolePanelWidget : public BasePanelWidget
{
    Q_OBJECT

public:
    explicit ConsolePanelWidget(const QString& title, QWidget* parent = nullptr);

signals:
    // Сигнал отправляется, когда пользователь нажимает Enter
    void commandEntered(const QString& command);

private slots:
    // Слот для обработки нажатия Enter
    void onReturnPressed();

private:
    QLineEdit* m_commandInput;
};
