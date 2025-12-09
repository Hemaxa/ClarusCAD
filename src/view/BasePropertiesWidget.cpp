#include "BasePropertiesWidget.h"
#include "BasePrimitive.h"
#include "ThemeManager.h"
#include "LineStyleManager.h"

#include <QPushButton>
#include <QColorDialog>
#include <QFormLayout>
#include <QStackedWidget>
#include <QGridLayout>
#include <QLabel>
#include <QSpacerItem>
#include <QVBoxLayout>
#include <QComboBox>
#include <QPainter>

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
    //populateLineTypeComboBox(); //заполнение при инициализации
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

void BasePropertiesWidget::setPrimitives(const QList<BasePrimitive*>& primitives)
{
    m_selectedPrimitives = primitives;

    // Режим создания нового (список пуст или содержит nullptr, но логика PropertiesPanelWidget передает сюда пустой список только при отмене)
    // Если primitives пуст, значит ничего не выбрано, но вызов идет через showPropertiesFor(nullptr), который сюда не доходит.
    // Логика "создания" идет через setPrimitives({nullptr})? Нет, PropertiesPanel вызывает setPrimitive(nullptr) -> setPrimitives({nullptr}).

    if (m_selectedPrimitives.isEmpty() || (m_selectedPrimitives.size() == 1 && !m_selectedPrimitives.first())) {
        // Режим создания
        m_currentPrimitive = nullptr;
        m_selectedColor = Qt::white;
        m_selectedLineTypeId = (int)LineType::SolidMain;
        m_applyButton->setText("Создать");
        populateLineTypeComboBox();
        updateColor(m_selectedColor, false);
        updateLineType(m_selectedLineTypeId, false);
    }
    else {
        // Режим редактирования
        m_currentPrimitive = m_selectedPrimitives.last();
        m_applyButton->setText(m_selectedPrimitives.size() > 1 ? QString("Обновить (%1)").arg(m_selectedPrimitives.size()) : "Обновить");

        // Проверка на разные свойства
        bool diffColors = false;
        bool diffTypes = false;

        QColor firstColor = m_selectedPrimitives.first()->getColor();
        int firstType = m_selectedPrimitives.first()->getLineType();

        for(auto* p : m_selectedPrimitives) {
            if(p->getColor() != firstColor) diffColors = true;
            if(p->getLineType() != firstType) diffTypes = true;
        }

        m_selectedColor = diffColors ? Qt::white : firstColor; //Если разные, храним "нейтральный" цвет, но покажем по-особому
        m_selectedLineTypeId = diffTypes ? -1 : firstType;

        populateLineTypeComboBox(); // Перезаполняем, чтобы стили обновились
        updateColor(m_selectedColor, diffColors);
        updateLineType(m_selectedLineTypeId, diffTypes);
    }

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
}

void BasePropertiesWidget::updateColor(const QColor& color, bool isMixed)
{
    m_selectedColor = color;

    if (isMixed) {
        // Если цвета разные - показываем градиент или "вопросительный" стиль
        m_colorButton->setStyleSheet("background-color: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 red, stop:0.5 green, stop:1 blue); border-radius: 12px; border: 1px solid gray;");
        m_colorButton->setText("?");
    } else {
        //обновление кружка цвета
        m_colorButton->setStyleSheet(QString("background-color: %1; border-radius: 12px; border: 1px solid gray;").arg(m_selectedColor.name()));
        m_colorButton->setText("");
    }
}

void BasePropertiesWidget::updateLineType(int typeId, bool isMixed)
{
    m_selectedLineTypeId = typeId;

    if (isMixed) {
        // Добавляем пункт "Разные" если его нет
        if (m_lineTypeComboBox->findText("Разные...") == -1) {
            m_lineTypeComboBox->insertItem(0, "Разные...", -1);
        }
        m_lineTypeComboBox->setCurrentIndex(0);
    } else {
        //обновление списка линий
        int index = m_lineTypeComboBox->findData(typeId);
        if (index != -1) {
            m_lineTypeComboBox->setCurrentIndex(index);
        }
    }
}

void BasePropertiesWidget::onColorButtonClicked()
{
    //вызов диалога выбора цвета
    QColor color = QColorDialog::getColor(m_selectedColor, this, "Выберите цвет");
    if (color.isValid()) {
        m_selectedColor = color;
        updateColor(m_selectedColor, false); // Теперь цвет единый
        emit colorChanged(m_selectedColor);
    }
}

void BasePropertiesWidget::onLineTypeBoxClicked(int index)
{
    if (index < 0) return;

    //получение типа линии из данных элемента
    int selectedTypeId = m_lineTypeComboBox->itemData(index).toInt();

    //Если выбрали "Разные...", ничего не делаем
    if (selectedTypeId == -1) return;

    m_selectedLineTypeId = selectedTypeId;

    //если активирован режим создания (нет m_currentPrimitive), то сразу отправляется сигнал для инструмента
    if (!m_currentPrimitive && m_selectedPrimitives.isEmpty()) {
        //Каст к LineType для совместимости с сигналами, если это стандартный тип
        //Если кастомный - механизм сигналов нужно обновлять, но пока кастуем (в инструменте нужно будет учитывать)
        emit lineTypeChanged(static_cast<LineType>(m_selectedLineTypeId));
    }
}

void BasePropertiesWidget::populateLineTypeComboBox()
{
    m_lineTypeComboBox->blockSignals(true);
    m_lineTypeComboBox->clear();
    QColor iconColor = ThemeManager::instance().getIconColor();

    // 1. Стандартные типы
    QMap<LineType, QString> lineTypeIcons;
    lineTypeIcons[LineType::SolidMain]      = ":/icons/icons/lines/solid-thick.svg";
    lineTypeIcons[LineType::SolidThin]      = ":/icons/icons/lines/solid.svg";
    lineTypeIcons[LineType::SolidWave]      = ":/icons/icons/lines/wave.svg";
    lineTypeIcons[LineType::SolidKink]      = ":/icons/icons/lines/zigzag.svg";
    lineTypeIcons[LineType::Dashed]         = ":/icons/icons/lines/dashed.svg";
    lineTypeIcons[LineType::DashDotThick]   = ":/icons/icons/lines/dash-dot-thick.svg";
    lineTypeIcons[LineType::DashDotThin]    = ":/icons/icons/lines/dash-dot.svg";
    lineTypeIcons[LineType::DashDotDot]     = ":/icons/icons/lines/dash-dot-dot.svg";

    for (auto it = lineTypeIcons.constBegin(); it != lineTypeIcons.constEnd(); ++it)
    {
        LineType type = it.key();
        QString path = it.value();
        QIcon icon = ThemeManager::colorizeSvg(path, iconColor);

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

    // 2. Пользовательские типы
    auto customStyles = LineStyleManager::instance().getCustomStyles();
    for(auto it = customStyles.begin(); it != customStyles.end(); ++it) {
        //Генерируем превью
        QPixmap pix(100, 20);
        pix.fill(Qt::transparent);
        QPainter p(&pix);
        LineStyleManager::instance().drawLine(p, QPointF(0, 10), QPointF(100, 10), it.key(), iconColor);

        m_lineTypeComboBox->addItem(QIcon(pix), it.value().name, it.key());
    }

    m_lineTypeComboBox->blockSignals(false);
}

void BasePropertiesWidget::updateColors()
{
    populateLineTypeComboBox(); //перерисовка иконок линий
    updatePrompt(); //перерисовка иконки подсказки
}

void BasePropertiesWidget::updatePrompt() {}
