#pragma once

#include <QWidget>

class BasePrimitive;

class BasePropertiesWidget : public QWidget
{
    Q_OBJECT

public:
    explicit BasePropertiesWidget(QWidget* parent = nullptr) : QWidget(parent) {}
    virtual ~BasePropertiesWidget() = default;
    virtual void setPrimitive(BasePrimitive* primitive) = 0;
};
