#include "ToolbarPanelWidget.h"

#include <QToolButton>
#include <QVBoxLayout>
#include <QIcon>
#include <QButtonGroup>

ToolbarPanelWidget::ToolbarPanelWidget(const QString& title, QWidget* parent) : BaseDockWidget(title, parent)
{
    //вертикальный шаблон компоновки, объекты прижимаются к верху
    auto* layout = new QVBoxLayout(canvas());
    layout->setAlignment(Qt::AlignTop);

    m_buttonGroup = new QButtonGroup(this); //создание группы кнопок
    m_buttonGroup->setExclusive(true); //возможность выбора только одной кнопки

    //создание и добавление кнопок в группу
    //текст описания, путь до иконки, горячая клавиша
    auto* createSegmentBtn = createToolButton("Отрезок [S]", ":/icons/icons/segment.svg", Qt::Key_S);

    //подключение сигнала
    connect(createSegmentBtn, &QToolButton::clicked, this, &ToolbarPanelWidget::segmentToolActivated);

    //минимальная ширина окна
    setMinimumWidth(200);
}

QToolButton* ToolbarPanelWidget::createToolButton(const QString& text, const QString& iconPath, const QKeySequence& shortcut)
{
    //создание кнопки
    auto* button = new QToolButton();
    button->setToolTip(text); //текст для кнопки
    button->setCheckable(true); //"залипающее" поведение кнопки

    //установка иконки
    button->setIcon(QIcon(iconPath));
    button->setIconSize(QSize(30, 30));

    button->setToolButtonStyle(Qt::ToolButtonIconOnly);
    button->setAutoRaise(true);

    //установка горячей клавиши
    button->setShortcut(shortcut);

    //добавление кнопки на панель и в группу
    qobject_cast<QVBoxLayout*>(canvas()->layout())->addWidget(button);
    m_buttonGroup->addButton(button);

    return button;
}
