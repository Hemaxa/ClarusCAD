#include "ViewportWidget.h"
#include <QPainter>

ViewportWidget::ViewportWidget(QWidget *parent) : QWidget(parent)
{
}

void ViewportWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setBrush(QColor(45, 45, 45));
    painter.drawRect(rect());
}
