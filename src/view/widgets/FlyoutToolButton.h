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

    /**
     * @brief Перекрасить основную иконку и элементы flyout в текущую тему.
     */
    void updateColors();

    /**
     * @brief Добавить режим во всплывающее меню.
     * @param modeId ID режима.
     * @param iconPath Путь к иконке.
     * @param tooltip Подсказка.
     */
    void addMode(int modeId, const QString& iconPath, const QString& tooltip);
    
    /**
     * @brief Установить текущий активный режим.
     * Менят иконку основной кнопки.
     */
    void setCurrentMode(int modeId);

    /**
     * @brief Получить текущий активный режим.
     */
    int getCurrentMode() const;

    /**
     * @brief Установить задержку перед появлением меню (в мс).
     */
    void setFlyoutDelay(int ms);
    
    /**
     * @brief Проверить, есть ли несколько режимов (нужно ли рисовать индикатор).
     */
    bool hasMultipleModes() const { return m_modes.size() > 1; }

signals:
    /**
     * @brief Сигнал выбора режима из меню.
     * @param modeId ID выбранного режима.
     */
    void modeActivated(int modeId);
    
    /**
     * @brief Сигнал клика по основной кнопке (без долгого нажатия).
     */
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
