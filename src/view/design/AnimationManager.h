//AnimationManager - класс, отвечающий за анимации в приложении

#pragma once

#include <QToolButton>
#include <QPropertyAnimation>
#include <QKeySequence>

class AnimationManager : public QToolButton
{
    Q_OBJECT
    // Объявляем наше собственное свойство для анимации
    Q_PROPERTY(qreal iconScale READ iconScale WRITE setIconScale)

public:
    explicit AnimationManager(const QString& iconPath, const QString& toolTip, const QKeySequence& shortcut, QWidget* parent = nullptr);

    // Геттер и сеттер для нашего нового свойства
    void updateIconColor(const QColor& color);
    void setIconScale(qreal scale);
    qreal iconScale() const;

protected:
    void enterEvent(QEnterEvent* event) override;
    void leaveEvent(QEvent* event) override;
    // Переопределяем метод отрисовки
    void paintEvent(QPaintEvent* event) override;

private:
    QPropertyAnimation* m_animation;
    qreal m_iconScale = 0.6; // Текущий масштаб иконки
    QPixmap m_pixmap; // Храним оригинальную иконку для быстрой отрисовки
    QString m_iconPath;
};
