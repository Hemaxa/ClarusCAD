#pragma once

#include <QDockWidget>

class BaseDockWidget : public QDockWidget
{
    Q_OBJECT

public:
    explicit BaseDockWidget(const QString& title, QWidget* parent = nullptr)
        : QDockWidget(title, parent) {}
    virtual ~BaseDockWidget() = default;
};
