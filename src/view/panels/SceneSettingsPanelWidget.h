#pragma once

#include "BasePanelWidget.h"

class QToolButton;

class SceneSettingsPanelWidget : public BasePanelWidget
{
    Q_OBJECT

public:
    explicit SceneSettingsPanelWidget(const QString& title, QWidget* parent = nullptr);

    void updateIcons();

signals:
    void gridSnapToggled(bool enabled);

private:
    QToolButton* createToolButton(const QString& text, const QString& iconPath, const QKeySequence& shortcut);
    QToolButton* m_gridSnapBtn;
};
