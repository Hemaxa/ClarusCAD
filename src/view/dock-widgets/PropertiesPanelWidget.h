#pragma once

#include "BaseDockWidget.h"
#include "PointCreationPrimitive.h"

class QStackedWidget;
class SegmentPropertiesWidget;
class BasePrimitive;

class PropertiesPanelWidget : public BaseDockWidget
{
    Q_OBJECT

public:
    explicit PropertiesPanelWidget(const QString& title, QWidget* parent = nullptr);

public slots:
    // Слот, который решает, какую "начинку" показать
    void showPropertiesFor(BasePrimitive* primitive);

signals:
    // Сигнал, который отправляется наверх в MainWindow
    void createSegmentRequested(const PointCreationPrimitive& start, const PointCreationPrimitive& end);

private:
    QStackedWidget* m_stack; // "Стопка" виджетов для переключения
    SegmentPropertiesWidget* m_segmentProperties;
    QWidget* m_emptyWidget; // Пустой виджет, когда ничего не выбрано
};
