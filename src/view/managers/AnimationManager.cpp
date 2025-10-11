#include "AnimationManager.h"
#include "ThemeManager.h"

#include <QPainter>
#include <QPaintEvent>

AnimationManager::AnimationManager(const QString& iconPath, const QString& toolTip, const QKeySequence& shortcut, QWidget* parent) : QToolButton(parent), m_iconPath(iconPath)
{
    //установка свойств для кнопки
    setToolTip(toolTip);
    setCheckable(true);
    setShortcut(shortcut);
    setFixedSize(42, 42); //размер кнопки

    m_currentIconScale = m_dfIconScale; //текущий масштаб равен масштабу без увеличения

    //получение цвета иконки из менеджера тем
    QColor iconColor = ThemeManager::instance().getIconColor();

    //извлекается и сохраняется растровое изображение, полученное из перекрашенной менеджером тем иконки
    m_pixmap = ThemeManager::colorizeSvg(iconPath, iconColor).pixmap(40, 40);

    //создается объект анимации (анимируется собственное свойство iconScale)
    m_animation = new QPropertyAnimation(this, "iconScale", this);
    m_animation->setDuration(100); //длительность анимации
    m_animation->setEasingCurve(QEasingCurve::OutCubic); //поведение анимации
}

void AnimationManager::updateIconColor(const QColor& color)
{
    //сохраненное изображение перезаписывается
    m_pixmap = ThemeManager::colorizeSvg(m_iconPath, color).pixmap(40, 40);
    update();
}

//запуск анимации увеличения при наведении мышью
void AnimationManager::enterEvent(QEnterEvent* event)
{
    m_animation->stop();
    m_animation->setEndValue(m_lgIconScale);
    m_animation->start();
    QToolButton::enterEvent(event);
}

//запуск анимации уменьшения при уведении мышь
void AnimationManager::leaveEvent(QEvent* event)
{
    m_animation->stop();
    m_animation->setEndValue(m_dfIconScale);
    m_animation->start();
    QToolButton::leaveEvent(event);
}

//собственный метод отрисовки
void AnimationManager::paintEvent(QPaintEvent* event)
{
    //вызывается родительский метод, чтобы нарисовался фон кнопки, рамка и т.д.
    QToolButton::paintEvent(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, true);

    //вычисляется размер иконки с учетом текущего масштаба
    int scaledSize = static_cast<int>(m_pixmap.width() * m_currentIconScale);

    //вычисляется позиция, чтобы иконка всегда была в центре виджета
    int x = (width() - scaledSize) / 2;
    int y = (height() - scaledSize) / 2;

    //отрисовка иконки
    painter.drawPixmap(x, y, scaledSize, scaledSize, m_pixmap);
}

void AnimationManager::setIconScale(qreal scale)
{
    if (m_currentIconScale != scale) {
        m_currentIconScale = scale;
        update();
    }
}

qreal AnimationManager::getIconScale() const
{
    return m_currentIconScale;
}
