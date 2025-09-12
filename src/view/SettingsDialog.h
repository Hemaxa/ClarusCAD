#pragma once

#include <QDialog>

class QComboBox;

class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsDialog(QWidget* parent = nullptr);

signals:
    // Сигнал, который отправляет путь к выбранной теме
    void themeChanged(const QString& themePath);

private slots:
    // Слот для применения настроек при нажатии "ОК"
    void applySettings();

private:
    QComboBox* m_themeComboBox;
};
