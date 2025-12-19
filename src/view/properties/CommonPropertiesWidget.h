//CommonPropertiesWidget - виджет общих свойств для мультивыделения разнотипных объектов

#pragma once

#include "BasePropertiesWidget.h"

// Упрощенный виджет для редактирования только цвета и типа линии
// Показывается когда выделены объекты разных типов (например, сегмент + окружность)
class CommonPropertiesWidget : public BasePropertiesWidget
{
    Q_OBJECT

public:
    explicit CommonPropertiesWidget(QWidget* parent = nullptr);

signals:
    // Сигнал применения общих свойств ко всем выделенным объектам
    void commonPropertiesApplied(const QColor& color, int lineTypeId);

protected:
    void updateFieldValues() override;

private slots:
    void onApplyClicked();
};
