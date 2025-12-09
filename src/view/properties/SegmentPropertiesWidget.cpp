#include "SegmentPropertiesWidget.h"
#include "SegmentPrimitive.h"
#include "ThemeManager.h"

#include <QFormLayout>
#include <QStackedWidget>
#include <QLineEdit>
#include <QDoubleValidator>
#include <QPushButton>

SegmentPropertiesWidget::SegmentPropertiesWidget(QWidget* parent) : BasePropertiesWidget(parent)
{
    //инициализация валидатора вводимых значений
    auto* validator = new QDoubleValidator(this);

    //заполнение панели декартовых координат
    auto* cartesianLayout = static_cast<QFormLayout*>(m_cartesianWidgets->layout());
    m_startXEdit = new QLineEdit("0.0");
    m_startYEdit = new QLineEdit("0.0");
    m_endXEdit = new QLineEdit("0.0");
    m_endYEdit = new QLineEdit("0.0");
    m_startXEdit->setValidator(validator);
    m_startYEdit->setValidator(validator);
    m_endXEdit->setValidator(validator);
    m_endYEdit->setValidator(validator);
    cartesianLayout->addRow("Первая точка X:", m_startXEdit);
    cartesianLayout->addRow("Первая точка Y:", m_startYEdit);
    cartesianLayout->addRow("Вторая точка X:", m_endXEdit);
    cartesianLayout->addRow("Вторая точка Y:", m_endYEdit);

    //заполнение панели полярных координат
    auto* polarLayout = static_cast<QFormLayout*>(m_polarWidgets->layout());
    m_startRadiusEdit = new QLineEdit("0.0");
    m_startAngleEdit = new QLineEdit("0.0");
    m_endRadiusEdit = new QLineEdit("0.0");
    m_endAngleEdit = new QLineEdit("0.0");
    m_startRadiusEdit->setValidator(validator);
    m_startAngleEdit->setValidator(validator);
    m_endRadiusEdit->setValidator(validator);
    m_endAngleEdit->setValidator(validator);
    polarLayout->addRow("Первая точка R:", m_startRadiusEdit);
    polarLayout->addRow("Первая точка A:", m_startAngleEdit);
    polarLayout->addRow("Вторая точка R:", m_endRadiusEdit);
    polarLayout->addRow("Вторая точка A:", m_endAngleEdit);

    //подключение сигнала от кнопки
    connect(m_applyButton, &QPushButton::clicked, this, &SegmentPropertiesWidget::onApplyButtonClicked);

    //установка системы координат по-умолчанию
    setCoordinateSystem(CoordinateSystemType::Cartesian);
}

void SegmentPropertiesWidget::setPrimitives(const QList<BasePrimitive*>& primitives)
{
    BasePropertiesWidget::setPrimitives(primitives);

    m_currentSegment = nullptr;
    if (m_currentPrimitive) {
        m_currentSegment = dynamic_cast<SegmentPrimitive*>(m_currentPrimitive);
    }

    updateFieldValues();
}

void SegmentPropertiesWidget::updateFieldValues()
{
    bool isCartesian = (m_selectedCoordSystem == CoordinateSystemType::Cartesian);

    // Если выбрано несколько объектов, геометрию можно показывать либо "разные", либо последнего.
    // Покажем последнего (как и было), но если объектов много - пользователь должен понимать, что геометрию
    // менять всем сразу опасно. Обычно в CAD геометрия блокируется при мультиселекте.
    // Но оставим как есть: показываем m_currentSegment (последний).

    //если существующий объект
    if (m_currentSegment) {
        const auto& startPoint = m_currentSegment->getStart();
        const auto& endPoint = m_currentSegment->getEnd();

        //значения в зависимости от системы координат
        if (isCartesian) {
            m_startXEdit->setText(QString::number(startPoint.getX()));
            m_startYEdit->setText(QString::number(startPoint.getY()));
            m_endXEdit->setText(QString::number(endPoint.getX()));
            m_endYEdit->setText(QString::number(endPoint.getY()));
        } else {
            m_startRadiusEdit->setText(QString::number(startPoint.getRadius()));
            m_startAngleEdit->setText(QString::number(startPoint.getAngle()));
            m_endRadiusEdit->setText(QString::number(endPoint.getRadius()));
            m_endAngleEdit->setText(QString::number(endPoint.getAngle()));
        }
    }
    //если новый объект
    else {
        m_startXEdit->setText("0.0");
        m_startYEdit->setText("0.0");
        m_endXEdit->setText("0.0");
        m_endYEdit->setText("0.0");
        m_startRadiusEdit->setText("0.0");
        m_startAngleEdit->setText("0.0");
        m_endRadiusEdit->setText("0.0");
        m_endAngleEdit->setText("0.0");
    }
}

void SegmentPropertiesWidget::updatePrompt()
{
    ThemeManager& tm = ThemeManager::instance();

    QMap<QString, QColor> colorMap;
    colorMap.insert("currentColor", tm.getIconColor());
    //colorMap.insert("@textColor", tm.getColor("textColor"));

    QPixmap originalPixmap = tm.colorizeSvg(":/promts/promts/segment-promt.svg", colorMap);
    if (originalPixmap.isNull()) {
        return;
    }

    //размер подсказки
    QPixmap scaledPixmap = originalPixmap.scaledToWidth(130, Qt::SmoothTransformation);

    m_leftColumn->setPixmap(scaledPixmap);
}


void SegmentPropertiesWidget::onApplyButtonClicked()
{
    PointPrimitive start, end;
    bool isCartesian = (m_selectedCoordSystem == CoordinateSystemType::Cartesian);

    //считывание текста из полей ввода в зависимости от системы координат
    if (isCartesian) {
        start.setX(m_startXEdit->text().toDouble());
        start.setY(m_startYEdit->text().toDouble());
        end.setX(m_endXEdit->text().toDouble());
        end.setY(m_endYEdit->text().toDouble());
    }
    else {
        start.setPolar(m_startRadiusEdit->text().toDouble(), m_startAngleEdit->text().toDouble());
        end.setPolar(m_endRadiusEdit->text().toDouble(), m_endAngleEdit->text().toDouble());
    }

    // Получаем выбранный тип линии. Если "Разные" (-1), берем старый.
    // Но здесь мы применяем изменения. Если пользователь нажал "Обновить", и тип "Разные",
    // скорее всего он не хочет менять тип линии у всех.

    // Важно: в MainWindow applySegmentChanges будет применяться к m_selectedPrimitives.
    // Нужно передать данные.
    // Для совместимости мы передаем m_currentSegment как "главный".

    // Трюк: LineType в сигнале - enum. Если у нас кастомный стиль (int > 1000),
    // сигнал не сможет его передать без static_cast, и получатель (MainWindow) должен быть готов к этому.

    // Если пользователь выбрал "Разные" (id == -1), нужно передать что-то, что скажет "не меняй".
    // Но текущий механизм этого не поддерживает напрямую через параметры метода.
    // Однако, MainWindow применяет цвет и линию из аргументов ко всем.
    // Если id == -1, мы можем передать текущий тип линии m_currentSegment-а, но тогда все станут как он.
    // Это ограничение текущей архитектуры сигнала.
    // Доработаем: если в комбобоксе "Разные", берем тип линии "главного" объекта, чтобы хоть что-то передать.
    // В идеале MainWindow должен смотреть: если в виджете "Разные", не менять.

    int typeToEmit = m_selectedLineTypeId;
    if(typeToEmit == -1 && m_currentSegment) typeToEmit = m_currentSegment->getLineType();
    if(typeToEmit == -1) typeToEmit = (int)LineType::SolidMain; // Fallback

    emit propertiesApplied(m_currentSegment, start, end, m_selectedColor, static_cast<LineType>(typeToEmit));
}
