//FlyoutToolButton - кнопка инструмента с выпадающим меню режимов построения
//При зажатии кнопки появляется панель с другими режимами

#pragma once

#include <QToolButton>
#include <QTimer>
#include <QVector>

class QHBoxLayout;
class QPaintEvent;

struct FlyoutMode {
    int modeId;
    QString iconPath;
    QString tooltip;
};

class FlyoutToolButton : public QToolButton
{
    Q_OBJECT

public:
    explicit FlyoutToolButton(QWidget* parent = nullptr);
    ~FlyoutToolButton();

    // Добавление режима в flyout панель
    void addMode(int modeId, const QString& iconPath, const QString& tooltip);
    
    // Установка текущего режима (меняет иконку основной кнопки)
    void setCurrentMode(int modeId);
    int getCurrentMode() const;

    // Задержка перед появлением flyout (мс)
    void setFlyoutDelay(int ms);
    
    // Есть ли несколько режимов (для отрисовки треугольника)
    bool hasMultipleModes() const { return m_modes.size() > 1; }

signals:
    // Сигнал выбора режима (id режима)
    void modeActivated(int modeId);
    
    // Сигнал клика по основной кнопке (без зажатия)
    void clicked();

protected:
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void leaveEvent(QEvent* event) override;
    void paintEvent(QPaintEvent* event) override;

private slots:
    void onFlyoutTimeout();
    void onModeButtonClicked();

private:
    void showFlyout();
    void hideFlyout();
    void updateButtonIcon(QToolButton* btn, const QString& iconPath);

    QVector<FlyoutMode> m_modes;
    int m_currentModeId = 0;
    
    QWidget* m_flyoutWidget = nullptr;
    QHBoxLayout* m_flyoutLayout = nullptr;
    
    QTimer* m_pressTimer = nullptr;
    int m_flyoutDelay = 300; // мс
    bool m_flyoutShown = false;
    bool m_wasLongPress = false;
};
