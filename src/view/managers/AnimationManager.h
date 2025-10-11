//AnimationManager - класс, отвечающий за анимации в приложении

#pragma once

#include <QToolButton>
#include <QPropertyAnimation>
#include <QKeySequence>

class AnimationManager : public QToolButton
{
    Q_OBJECT

    //объявляется собственное свойство для анимации (будет вызывать метод iconScale)
    Q_PROPERTY(qreal iconScale READ getIconScale WRITE setIconScale)

public:
    //конструктор
    explicit AnimationManager(const QString& iconPath, const QString& toolTip, const QKeySequence& shortcut, QWidget* parent = nullptr);

    //метод изменения цвета иконки
    void updateIconColor(const QColor& color);

    //геттер и сеттер
    void setIconScale(qreal scale);
    qreal getIconScale() const;

protected:
    //переопределение методов работы с мышью
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
