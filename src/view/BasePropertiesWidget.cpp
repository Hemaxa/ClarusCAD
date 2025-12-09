#include "BasePropertiesWidget.h"
#include "BasePrimitive.h"
#include "ThemeManager.h"

#include <QPushButton>
#include <QColorDialog>
#include <QFormLayout>
#include <QStackedWidget>
#include <QGridLayout>
#include <QLabel>
#include <QSpacerItem>
#include <QVBoxLayout>
#include <QComboBox>

BasePropertiesWidget::BasePropertiesWidget(QWidget* parent) : QWidget(parent)
{
    //установка системы координат, цвета и типа линии по-умолчанию
    m_selectedCoordSystem = CoordinateSystemType::Cartesian;

    //создание главной сетки
    auto* mainLayout = new QGridLayout(this);
    mainLayout->setContentsMargins(10, 10, 10, 10);

    //1) левая колонка (подсказка)
    m_leftColumn = new QLabel();
    m_leftColumn->setAlignment(Qt::AlignTop | Qt::AlignHCenter);
    m_leftColumn->setContentsMargins(10, 20, 10, 10);

    //2) правая колонка (общие параметры)
    m_rightColumn = new QWidget();
    auto* rightLayout = new QFormLayout(m_rightColumn);
    rightLayout->setAlignment(Qt::AlignTop | Qt::AlignHCenter);
    rightLayout->setContentsMargins(0, 15, 10, 0);

    //создание кнопки изменения цвета
    m_colorButton = new QPushButton();
    m_colorButton->setFixedSize(25, 25);
    m_colorButton->setCursor(Qt::PointingHandCursor);
    connect(m_colorButton, &QPushButton::clicked, this, &BasePropertiesWidget::onColorButtonClicked);
    rightLayout->addRow("Цвет:", m_colorButton);

    //cоздание списка выбора типа линии
    m_lineTypeComboBox = new QComboBox();
    m_lineTypeComboBox->setFixedWidth(130); //ширина поля
    m_lineTypeComboBox->setIconSize(QSize(100, 20)); //размер иконок
    m_lineTypeComboBox->setObjectName("LineTypeComboBox");
    m_lineTypeComboBox->setCursor(Qt::PointingHandCursor);
    populateLineTypeComboBox(); //заполнение
    connect(m_lineTypeComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &BasePropertiesWidget::onLineTypeBoxClicked);
    rightLayout->addRow("Тип линии:", m_lineTypeComboBox);

    //3) центральная колонка (сменяемые параметры)
    m_centralColumn = new QWidget();
    auto* centralLayout = new QVBoxLayout(m_centralColumn);
    centralLayout->setAlignment(Qt::AlignTop | Qt::AlignHCenter);
    centralLayout->setContentsMargins(0, 0, 0, 0);

    //создание и заполнение общей панели с параметрами внутри центральной колонки
    m_paramsStack = new QStackedWidget();
    m_cartesianWidgets = new QWidget();
    m_polarWidgets = new QWidget();

    auto* cartesianLayout = new QFormLayout(m_cartesianWidgets);
    cartesianLayout->setContentsMargins(0, 15, 10, 0);

    auto* polarLayout = new QFormLayout(m_polarWidgets);
    polarLayout->setContentsMargins(0, 15, 10, 0);

    m_paramsStack->addWidget(m_cartesianWidgets);
    m_paramsStack->addWidget(m_polarWidgets);
    centralLayout->addWidget(m_paramsStack);

    //создание кнопки "Создать/Обновить"
    m_applyButton = new QPushButton();
    m_applyButton->setFixedSize(120, 30);

    //cборка всей сетки
    mainLayout->addWidget(m_leftColumn, 0, 0, Qt::AlignHCenter);
    mainLayout->addWidget(m_centralColumn, 0, 1, Qt::AlignHCenter);
    mainLayout->addWidget(m_rightColumn, 0, 2, Qt::AlignHCenter);

    mainLayout->addWidget(m_applyButton, 1, 0, 1, 3, Qt::AlignHCenter);

    //равномерное растягивание колонок
    mainLayout->setColumnStretch(0, 1);
    mainLayout->setColumnStretch(1, 1);
    mainLayout->setColumnStretch(2, 1);
}

void BasePropertiesWidget::setPrimitive(BasePrimitive* primitive)
{
    m_currentPrimitive = primitive;

    //режим редактирования
    if (m_currentPrimitive) {
        m_selectedColor = m_currentPrimitive->getColor();
        m_selectedLineType = m_currentPrimitive->getLineType();
        m_applyButton->setText("Обновить");
    }
    //режим создания
    else
    {
        m_selectedColor = Qt::white;
        m_selectedLineType = LineType::SolidMain;
        m_applyButton->setText("Создать");
    }

    updateColor(m_selectedColor);
    updateLineType(m_selectedLineType);
    updateFieldValues();
    updatePrompt();
}

void BasePropertiesWidget::setCoordinateSystem(CoordinateSystemType type)
{
    //получение типа системы координат
    m_selectedCoordSystem = type;

    //декартова система координат
    if (m_selectedCoordSystem == CoordinateSystemType::Cartesian)
    {
        m_paramsStack->setCurrentWidget(m_cartesianWidgets);
    }
    //полярная система координат
    else
    {
        m_paramsStack->setCurrentWidget(m_polarWidgets);
    }

    updateFieldValues();
    //updatePrompt();
}

void BasePropertiesWidget::updateColor(const QColor& color)
{
    m_selectedColor = color;

    //обновление кружка цвета
    m_colorButton->setStyleSheet(QString("background-color: %1; border-radius: 12px; border: 1px solid gray;").arg(m_selectedColor.name()));
}

void BasePropertiesWidget::updateLineType(LineType type)
{
    m_selectedLineType = type;

    //обновление списка линий
    int index = m_lineTypeComboBox->findData(static_cast<int>(m_selectedLineType));
    if (index != -1)
    {
        m_lineTypeComboBox->setCurrentIndex(index);
    }
}

void BasePropertiesWidget::onColorButtonClicked()
{
    //вызов диалога выбора цвета
    QColor color = QColorDialog::getColor(m_selectedColor, this, "Выберите цвет");
    if (color.isValid()) {
        m_selectedColor = color;
        updateColor(m_selectedColor);
        emit colorChanged(m_selectedColor);
    }
}

void BasePropertiesWidget::onLineTypeBoxClicked(int index)
{
    if (index < 0) return;

    //получение типа линии из данных элемента
    LineType selectedType = static_cast<LineType>(m_lineTypeComboBox->itemData(index).toInt());
    m_selectedLineType = selectedType;

    //если активирован режим создания (нет m_currentPrimitive), то сразу отправляется сигнал для инструмента
    if (!m_currentPrimitive) {
        emit lineTypeChanged(m_selectedLineType);
    }
    //иначе сигнал не отправляется, тип линии применится по кнопке "Обновить"

    //уведомление (для инструмента создания)
    emit lineTypeChanged(m_selectedLineType);
}

void BasePropertiesWidget::populateLineTypeComboBox()
{
    m_lineTypeComboBox->clear();
    QColor iconColor = ThemeManager::instance().getIconColor();

    //карта <тип линии, путь к иконке>
    QMap<LineType, QString> lineTypeIcons;
    lineTypeIcons[LineType::SolidMain]      = ":/icons/icons/lines/solid-thick.svg"; // Основная
    lineTypeIcons[LineType::SolidThin]      = ":/icons/icons/lines/solid.svg";       // Тонкая
    lineTypeIcons[LineType::SolidWave]      = ":/icons/icons/lines/wave.svg";        // Волнистая (NEW)
    lineTypeIcons[LineType::SolidKink]      = ":/icons/icons/lines/zigzag.svg";      // С изломами (NEW)
    lineTypeIcons[LineType::Dashed]         = ":/icons/icons/lines/dashed.svg";
    lineTypeIcons[LineType::DashDotThick]   = ":/icons/icons/lines/dash-dot-thick.svg"; // Утолщенная (NEW)
    lineTypeIcons[LineType::DashDotThin]    = ":/icons/icons/lines/dash-dot.svg";
    lineTypeIcons[LineType::DashDotDot]     = ":/icons/icons/lines/dash-dot-dot.svg";

    for (auto it = lineTypeIcons.constBegin(); it != lineTypeIcons.constEnd(); ++it)
    {
        LineType type = it.key();
        QString path = it.value();
        QIcon icon = ThemeManager::colorizeSvg(path, iconColor);

        // Получаем понятное имя для пользователя
        QString name;
        switch(type) {
        case LineType::SolidMain: name = "Сплошная основная"; break;
        case LineType::SolidThin: name = "Сплошная тонкая"; break;
        case LineType::SolidWave: name = "Сплошная волнистая"; break;
        case LineType::Dashed:    name = "Штриховая"; break;
        case LineType::DashDotThick: name = "Штрихпунктирная (толстая)"; break;
        case LineType::DashDotThin:  name = "Штрихпунктирная (тонкая)"; break;
        case LineType::DashDotDot:   name = "С двумя точками"; break;
        case LineType::SolidKink:    name = "С изломами"; break;
        default: name = "Линия"; break;
        }

        m_lineTypeComboBox->addItem(icon, name, static_cast<int>(type));
    }
}

void BasePropertiesWidget::updateColors()
{
    populateLineTypeComboBox(); //перерисовка иконок линий
    updatePrompt(); //перерисовка иконки подсказки
}

void BasePropertiesWidget::updatePrompt() {}
