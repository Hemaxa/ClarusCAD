#pragma once
#include "BasePropertiesWidget.h"
#include "PointPrimitive.h"
#include <QLineEdit>
#include <QComboBox>
#include <QStackedWidget>

class RectanglePrimitive;

class RectanglePropertiesWidget : public BasePropertiesWidget
{
    Q_OBJECT
public:
    explicit RectanglePropertiesWidget(QWidget* parent = nullptr);
    void setPrimitives(const QList<BasePrimitive*>& primitives) override;

signals:
    // Сигнал передает все параметры прямоугольника включая скругление
    void propertiesApplied(RectanglePrimitive* rect, const PointPrimitive& center, 
                           double w, double h, double rotation, 
                           CornerType cornerType, double cornerRadius,
                           const QColor& color, LineType type);

private slots:
    void onApplyButtonClicked();
    void onModeChanged(int index);

private:
    void updateFieldValues() override;

    RectanglePrimitive* m_currentRect = nullptr;

    // Выбор способа построения
    QComboBox* m_modeComboBox;
    QStackedWidget* m_modeStack;

    // Поля Декартовой системы (Cartesian)
    QLineEdit* m_centerX;
    QLineEdit* m_centerY;

    // Поля Полярной системы (Polar)
    QLineEdit* m_centerRadius;
    QLineEdit* m_centerAngle;

    // Общие поля (размеры и поворот не зависят от системы координат положения)
    QLineEdit* m_width;
    QLineEdit* m_height;
    QLineEdit* m_rotation;

    // Скругление углов
    QComboBox* m_cornerTypeCombo;
    QLineEdit* m_cornerRadiusEdit;
};
