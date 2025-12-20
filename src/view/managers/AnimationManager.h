//AnimationManager - класс, отвечающий за анимации в приложении

#pragma once

#include <QToolButton>
#include <QPropertyAnimation>
#include <QKeySequence>

class AnimationManager : public QToolButton
{
    Q_OBJECT

    // Объявляется собственное свойство для анимации (будет вызывать метод iconScale)
    Q_PROPERTY(qreal iconScale READ getIconScale WRITE setIconScale)

public:
    /**
     * @brief Конструктор.
     * @param iconPath Путь к иконке.
     * @param toolTip Подсказка.
     * @param shortcut Горячая клавиша.
     * @param isCheckable Можно ли "нажать" (toggle).
     * @param parent Родительский виджет.
     */
    explicit AnimationManager(const QString& iconPath, const QString& toolTip, const QKeySequence& shortcut, bool isCheckable = true, QWidget* parent = nullptr);

    /**
     * @brief Обновить цвет иконки.
     * @param color Новый цвет.
     */
    void updateIconColor(const QColor& color);

    /**
     * @brief Установить масштаб отображения иконки.
     */
    void setIconScale(qreal scale);

    /**
     * @brief Получить текущий масштаб отображения иконки.
     */
    qreal getIconScale() const;

protected:
    // Переопределение методов работы с мышью
    void enterEvent(QEnterEvent* event) override;
    void leaveEvent(QEvent* event) override;

    //переопределение метода отрисовки
    void paintEvent(QPaintEvent* event) override;

private:
    QPropertyAnimation* m_animation; //указатель на анимируемый объект

    qreal m_currentIconScale; //текущий масштаб иконки, который меняется во время анимации
    const qreal m_dfIconScale = 0.65; //обычный масштаб иконки
    const qreal m_lgIconScale = 0.75; //увеличенный масштаб иконки

    QPixmap m_pixmap; //поле хранения оригинальной иконки (для оптимизации)
    QString m_iconPath; //путь к файлу иконки
};
