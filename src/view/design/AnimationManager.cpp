// src/view/design/AnimationManager.cpp

#include "AnimationManager.h"
#include "ThemeManager.h"

#include <QPainter>
#include <QPaintEvent>

AnimationManager::AnimationManager(const QString& iconPath, const QString& toolTip, const QKeySequence& shortcut, QWidget* parent)
    : QToolButton(parent), m_iconPath(iconPath)
{
    setToolTip(toolTip);
    setCheckable(true);
    setShortcut(shortcut);
    // Устанавливаем фиксированный размер для всего виджета, чтобы он не "прыгал" в компоновке
    setFixedSize(42, 42);

    // Получаем цвет, создаем иконку и сохраняем ее как QPixmap
    QColor iconColor = ThemeManager::instance().getIconColor();
    // Мы больше не используем setIcon(), так как будем рисовать сами
    m_pixmap = ThemeManager::colorizeSvgIcon(iconPath, iconColor).pixmap(30, 30);

    // Анимируем наше новое свойство "iconScale"
    m_animation = new QPropertyAnimation(this, "iconScale", this);
    m_animation->setDuration(120); // Чуть быстрее для отзывчивости
    m_animation->setEasingCurve(QEasingCurve::OutCubic);
}

void AnimationManager::updateIconColor(const QColor& color)
{
    m_pixmap = ThemeManager::colorizeSvgIcon(m_iconPath, color).pixmap(30, 30);
    update(); // Перерисовываем виджет с новой иконкой
}

// Сеттер для свойства iconScale
void AnimationManager::setIconScale(qreal scale)
{
    if (m_iconScale != scale) {
        m_iconScale = scale;
        update(); // Говорим виджету, что его нужно перерисовать
    }
}

qreal AnimationManager::iconScale() const
{
    return m_iconScale;
}

// Запускаем анимацию увеличения
void AnimationManager::enterEvent(QEnterEvent* event)
{
    m_animation->stop();
    m_animation->setEndValue(0.7);
    m_animation->start();
    QToolButton::enterEvent(event);
}

// Запускаем анимацию уменьшения
void AnimationManager::leaveEvent(QEvent* event)
{
    m_animation->stop();
    m_animation->setEndValue(0.6);
    m_animation->start();
    QToolButton::leaveEvent(event);
}

// Наш собственный метод отрисовки
void AnimationManager::paintEvent(QPaintEvent* event)
{
    // Сначала вызываем родительский метод, чтобы нарисовался фон кнопки, рамка и т.д.
    QToolButton::paintEvent(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    painter.setRenderHint(QPainter::SmoothPixmapTransform, true);

    // Вычисляем размер иконки с учетом текущего масштаба
    int scaledSize = static_cast<int>(m_pixmap.width() * m_iconScale);

    // Вычисляем позицию, чтобы иконка всегда была в центре виджета
    int x = (width() - scaledSize) / 2;
    int y = (height() - scaledSize) / 2;

    // Рисуем нашу иконку (m_pixmap) в вычисленном прямоугольнике
    painter.drawPixmap(x, y, scaledSize, scaledSize, m_pixmap);
}
