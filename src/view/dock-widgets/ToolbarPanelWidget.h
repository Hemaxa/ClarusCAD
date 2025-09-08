#pragma once

#include "BaseDockWidget.h"

class ToolbarPanelWidget : public BaseDockWidget
{
    Q_OBJECT
public:
    explicit ToolbarPanelWidget(const QString& title, QWidget* parent = nullptr);

signals:
    void segmentToolActivated();
};
