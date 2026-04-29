#include "FlyoutToolButton.h"
#include "ThemeManager.h"

#include <QHBoxLayout>
#include <QMouseEvent>
#include <QApplication>
#include <QScreen>
#include <QPainter>

FlyoutToolButton::FlyoutToolButton(QWidget* parent)
    : QToolButton(parent)
{
    setCheckable(true);
    setCursor(Qt::PointingHandCursor);
    
    // Таймер для определения зажатия
    m_pressTimer = new QTimer(this);
    m_pressTimer->setSingleShot(true);
    connect(m_pressTimer, &QTimer::timeout, this, &FlyoutToolButton::onFlyoutTimeout);
    
    // Создание flyout виджета (изначально скрыт)
    m_flyoutWidget = new QWidget(nullptr, Qt::Popup | Qt::FramelessWindowHint);
    m_flyoutWidget->setAttribute(Qt::WA_TranslucentBackground);
    m_flyoutWidget->setObjectName("FlyoutPanel");
    
    m_flyoutLayout = new QHBoxLayout(m_flyoutWidget);
    m_flyoutLayout->setContentsMargins(4, 4, 4, 4);
    m_flyoutLayout->setSpacing(4);
}

FlyoutToolButton::~FlyoutToolButton()
{
    delete m_flyoutWidget;
}

void FlyoutToolButton::addMode(int modeId, const QString& iconPath, const QString& tooltip)
{
    FlyoutMode mode;
    mode.modeId = modeId;
    mode.iconPath = iconPath;
    mode.tooltip = tooltip;
    m_modes.append(mode);
    
    // Создание кнопки для этого режима в flyout
    auto* btn = new QToolButton(m_flyoutWidget);
    btn->setObjectName("FlyoutModeButton");
    btn->setToolTip(tooltip);
    btn->setProperty("modeId", modeId);
    btn->setCursor(Qt::PointingHandCursor);
    btn->setFixedSize(36, 36);
    btn->setIconSize(QSize(24, 24));
    
    updateButtonIcon(btn, iconPath);
    
    connect(btn, &QToolButton::clicked, this, &FlyoutToolButton::onModeButtonClicked);
    m_flyoutLayout->addWidget(btn);
    
    // Первый добавленный режим становится текущим
    if (m_modes.size() == 1) {
        setCurrentMode(modeId);
    }
}

void FlyoutToolButton::setCurrentMode(int modeId)
{
    m_currentModeId = modeId;
    
    // Найти иконку для этого режима и установить на основную кнопку
    for (const auto& mode : m_modes) {
        if (mode.modeId == modeId) {
            updateButtonIcon(this, mode.iconPath);
            setToolTip(mode.tooltip);
            break;
        }
    }
}

int FlyoutToolButton::getCurrentMode() const
{
    return m_currentModeId;
}

void FlyoutToolButton::updateColors()
{
    for (const auto& mode : m_modes) {
        if (mode.modeId == m_currentModeId) {
            updateButtonIcon(this, mode.iconPath);
            break;
        }
    }

    const auto buttons = m_flyoutWidget->findChildren<QToolButton*>();
    for (QToolButton* button : buttons) {
        if (!button) {
            continue;
        }

        const int modeId = button->property("modeId").toInt();
        for (const auto& mode : m_modes) {
            if (mode.modeId == modeId) {
                updateButtonIcon(button, mode.iconPath);
                break;
            }
        }
    }

    update();
}

void FlyoutToolButton::setFlyoutDelay(int ms)
{
    m_flyoutDelay = ms;
}

void FlyoutToolButton::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        m_wasLongPress = false;
        
        // Если есть режимы для выбора, запускаем таймер
        if (m_modes.size() > 1) {
            m_pressTimer->start(m_flyoutDelay);
        }
    }
    QToolButton::mousePressEvent(event);
}

void FlyoutToolButton::mouseReleaseEvent(QMouseEvent* event)
{
    m_pressTimer->stop();
    
    if (event->button() == Qt::LeftButton) {
        if (m_flyoutShown) {
            // Если flyout был показан, скрываем его
            hideFlyout();
        } else if (!m_wasLongPress) {
            // Обычный клик — активируем текущий режим
            emit modeActivated(m_currentModeId);
            emit clicked();
        }
    }
    
    QToolButton::mouseReleaseEvent(event);
}

void FlyoutToolButton::leaveEvent(QEvent* event)
{
    // Если курсор вышел с кнопки и flyout не показан, останавливаем таймер
    if (!m_flyoutShown) {
        m_pressTimer->stop();
    }
    QToolButton::leaveEvent(event);
}

void FlyoutToolButton::onFlyoutTimeout()
{
    m_wasLongPress = true;
    showFlyout();
}

void FlyoutToolButton::onModeButtonClicked()
{
    auto* btn = qobject_cast<QToolButton*>(sender());
    if (!btn) return;
    
    int modeId = btn->property("modeId").toInt();
    setCurrentMode(modeId);
    hideFlyout();
    
    emit modeActivated(modeId);
}

void FlyoutToolButton::showFlyout()
{
    if (m_modes.size() <= 1) return;
    
    // Позиционируем flyout справа от кнопки
    QPoint globalPos = mapToGlobal(QPoint(0, 0));
    int flyoutHeight = m_flyoutWidget->sizeHint().height();
    
    // Размещаем справа от кнопки, выравнивая по центру по вертикали
    int x = globalPos.x() + width() + 4;
    int y = globalPos.y() + (height() - flyoutHeight) / 2;
    
    // Проверка что не выходит за пределы экрана
    QScreen* screen = QApplication::screenAt(globalPos);
    if (screen) {
        QRect screenGeom = screen->availableGeometry();
        int flyoutWidth = m_flyoutWidget->sizeHint().width();
        if (x + flyoutWidth > screenGeom.right()) {
            x = globalPos.x() - flyoutWidth - 4; // Показать слева если не влезает
        }
    }
    
    m_flyoutWidget->move(x, y);
    m_flyoutWidget->show();
    m_flyoutShown = true;
}

void FlyoutToolButton::hideFlyout()
{
    m_flyoutWidget->hide();
    m_flyoutShown = false;
}

void FlyoutToolButton::updateButtonIcon(QToolButton* btn, const QString& iconPath)
{
    QColor iconColor = ThemeManager::instance().getIconColor();
    QIcon icon = ThemeManager::colorizeSvg(iconPath, iconColor);
    
    if (icon.isNull()) {
        // Если иконка не найдена, показать placeholder
        btn->setText("?");
    } else {
        btn->setIcon(icon);
    }
}

void FlyoutToolButton::paintEvent(QPaintEvent* event)
{
    // Сначала рисуем стандартную кнопку
    QToolButton::paintEvent(event);
    
    // Если есть несколько режимов — рисуем треугольник в правом нижнем углу
    if (m_modes.size() > 1) {
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);
        
        // Маленький треугольник (6x6) в правом нижнем углу
        int size = 6;
        int margin = 3;
        int x = width() - size - margin;
        int y = height() - size - margin;
        
        QPolygonF triangle;
        triangle << QPointF(x, y + size)           // Левый нижний
                 << QPointF(x + size, y + size)    // Правый нижний  
                 << QPointF(x + size, y);          // Правый верхний
        
        painter.setPen(Qt::NoPen);
        painter.setBrush(ThemeManager::instance().getIconColor());
        painter.drawPolygon(triangle);
    }
}
