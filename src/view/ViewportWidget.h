#pragma once
#include <QWidget>

class ViewportWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ViewportWidget(QWidget *parent = nullptr);

protected:
    void paintEvent(QPaintEvent *event) override;
};
