//DimensionStyle - стиль оформления размерных линий

#pragma once

#include <QColor>
#include <QString>

enum class DimensionArrowType {
    ClosedFilled,
    ClosedOpen,
    Slash
};

/**
 * @brief Структура для хранения стилей размерных линий по стандарту ЕСКД
 */
struct DimensionStyle {
    QString fontFamily = "Monaco";                   ///< Шрифт размерного текста
    double textHeight = 14.0;                        ///< Высота текста в экранно-устойчивых единицах
    double textGap = 10.0;                           ///< Отступ текста от размерной линии
    double textAlongLineOffset = 0.0;                ///< Смещение текста вдоль размерной линии
    double arrowSize = 10.0;                         ///< Длина стрелки
    DimensionArrowType arrowType = DimensionArrowType::ClosedFilled;
    bool arrowFilled = true;                         ///< Заполнение стрелки
    double extensionLineOffset = 8.0;                ///< Отступ выносной линии от объекта
    double extensionLineExtend = 10.0;               ///< Выход выносной линии за размерную
    double dimensionLineExtension = 0.0;             ///< Выход размерной линии за выносные
    int extensionLineTypeId = 0;                     ///< Тип выносных линий
    int dimensionLineTypeId = 0;                     ///< Тип размерной линии
    QColor textColor = Qt::white;                    ///< Цвет текста
    QColor extensionLineColor = Qt::white;           ///< Цвет выносных линий
    QColor dimensionLineColor = Qt::white;           ///< Цвет размерной линии
};
