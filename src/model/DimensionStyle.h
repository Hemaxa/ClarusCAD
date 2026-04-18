//DimensionStyle - стиль оформления размерных линий

#pragma once

#include <QColor>

/**
 * @brief Структура для хранения стилей размерных линий по стандарту ЕСКД
 */
struct DimensionStyle {
    double textHeight = 3.5;                ///< Высота текста
    double arrowSize = 2.5;                 ///< Длина стрелки
    double extensionLineOffset = 1.0;       ///< Отступ выносной линии от объекта
    double extensionLineExtend = 2.0;       ///< Выход выносной линии за размерную
    QColor textColor = Qt::white;           ///< Цвет текста
    QColor dimensionLineColor = Qt::white;  ///< Цвет размерной линии
};
