// src/view/panels/NavigationPanelWidget.cpp

#include "NavigationPanelWidget.h"
#include "AnimationManager.h"

#include <QGridLayout>
#include <QKeySequence>

NavigationPanelWidget::NavigationPanelWidget(const QString& title, QWidget* parent) : BasePanelWidget(title, parent)
{
    //сетка для всех кнопок
    auto* layout = new QGridLayout(canvas());
    layout->setContentsMargins(10, 10, 10, 10);

    //--- Создание кнопок ---
    m_zoomInBtn = new AnimationManager(":/icons/icons/navigation/zoom_in.svg", "Приблизить [Ctrl+]", QKeySequence::ZoomIn, false);
    m_zoomOutBtn = new AnimationManager(":/icons/icons/navigation/zoom_out.svg", "Отдалить [Ctrl-]", QKeySequence::ZoomOut, false);
    m_zoomExtentsBtn = new AnimationManager(":/icons/icons/navigation/zoom_extents.svg", "Вписать все [F]", Qt::Key_F, false);

    // Новые кнопки (горячие клавиши [ и ] как в других CAD)
    m_rotateCWBtn = new AnimationManager(":/icons/icons/navigation/rotate_left.svg", "Повернуть против часовой [}]", Qt::Key_BracketRight, false);
    m_rotateCCWBtn = new AnimationManager(":/icons/icons/navigation/rotate_right.svg", "Повернуть по часовой [{]", Qt::Key_BracketLeft, false);

    //--- Добавление в шаблон ---
    // (Расположим их компактно 2x3)
    layout->addWidget(m_rotateCCWBtn,   0, 0);
    layout->addWidget(m_rotateCWBtn,    0, 1);
    layout->addWidget(m_zoomInBtn,      1, 0);
    layout->addWidget(m_zoomOutBtn,     1, 1);
    layout->addWidget(m_zoomExtentsBtn, 2, 0, 1, 2); // Растянем на 2 колонки

    //растягивание
    layout->setColumnStretch(2, 1);
    layout->setRowStretch(3, 1);

    //--- Подключение сигналов ---
    connect(m_zoomInBtn, &QToolButton::clicked, this, &NavigationPanelWidget::zoomInClicked);
    connect(m_zoomOutBtn, &QToolButton::clicked, this, &NavigationPanelWidget::zoomOutClicked);
    connect(m_zoomExtentsBtn, &QToolButton::clicked, this, &NavigationPanelWidget::zoomExtentsClicked);
    connect(m_rotateCWBtn, &QToolButton::clicked, this, &NavigationPanelWidget::rotateCWClicked);
    connect(m_rotateCCWBtn, &QToolButton::clicked, this, &NavigationPanelWidget::rotateCCWClicked);

    //минимальная ширина окна
    setMinimumWidth(160);
}
