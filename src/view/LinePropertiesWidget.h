#pragma once

#include <QWidget>
#include "Point.h"

class QLineEdit;

class LinePropertiesWidget : public QWidget
{
    Q_OBJECT

public:
    explicit LinePropertiesWidget(QWidget* parent = nullptr);

signals:
    // Сигнал, который будет отправлен при нажатии кнопки "Создать"
    void createSegmentRequested(const Point& start, const Point& end);

private slots:
    void onCreateButtonClicked();

private:
    QLineEdit* m_startXEdit;
    QLineEdit* m_startYEdit;
    QLineEdit* m_endXEdit;
    QLineEdit* m_endYEdit;
};
