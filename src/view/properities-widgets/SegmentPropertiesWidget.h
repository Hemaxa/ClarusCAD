#pragma once
#include "BasePropertiesWidget.h"
#include "PointCreationPrimitive.h"

class QLineEdit;
class SegmentCreationPrimitive;

class SegmentPropertiesWidget : public BasePropertiesWidget
{
    Q_OBJECT

signals:
    // Сигнал, который отправляется при нажатии кнопки "Создать"
    void createSegmentRequested(const PointCreationPrimitive& start, const PointCreationPrimitive& end);

private slots:
    // Слот для обработки нажатия кнопки
    void onCreateButtonClicked();

public:
    explicit SegmentPropertiesWidget(QWidget* parent = nullptr);
    void setPrimitive(BasePrimitive* primitive) override;

private:
    SegmentCreationPrimitive* m_currentSegment = nullptr;
    QLineEdit* m_startXEdit;
    QLineEdit* m_startYEdit;
    QLineEdit* m_endXEdit;
    QLineEdit* m_endYEdit;
};
