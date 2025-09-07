#pragma once

#include <QWidget>
#include <QButtonGroup>

class QToolButton;

class ToolbarPanel : public QWidget
{
    Q_OBJECT
public:
    explicit ToolbarPanel(QWidget* parent = nullptr);

signals:
    void createLineToolActivated();
    // Сигналы для других инструментов...

private:
    QButtonGroup* m_toolGroup;
};
