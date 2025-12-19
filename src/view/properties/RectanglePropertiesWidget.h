//RectanglePropertiesWidget - виджет свойств примитива "Прямоугольник"

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

    // Страница 0: Две точки
    QLineEdit* m_p1X;
    QLineEdit* m_p1Y;
    QLineEdit* m_p2X;
    QLineEdit* m_p2Y;

    // Страница 1: Центр + размер
    QLineEdit* m_centerX;
    QLineEdit* m_centerY;
    QLineEdit* m_width;
    QLineEdit* m_height;

    // Страница 2: Точка + размер
    QLineEdit* m_pointX;
    QLineEdit* m_pointY;
    QLineEdit* m_widthPS;
    QLineEdit* m_heightPS;

    // Общие поля
    QLineEdit* m_rotation;

    // Поля Полярной системы
    QLineEdit* m_centerRadius;
    QLineEdit* m_centerAngle;

    // Скругление углов
    QComboBox* m_cornerTypeCombo;
    QLineEdit* m_cornerRadiusEdit;
};
