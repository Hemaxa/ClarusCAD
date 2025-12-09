#include "LineStyleManager.h"
#include <QtMath>
#include <QPainter>
#include <QPainterPath>

LineStyleManager& LineStyleManager::instance()
{
    static LineStyleManager manager;
    return manager;
}

LineStyleManager::LineStyleManager(QObject* parent) : QObject(parent) {}

void LineStyleManager::setBaseLineThickness(double thickness)
{
    if (thickness < 1.0) thickness = 1.0;
    if (m_baseThickness != thickness) {
        m_baseThickness = thickness;
        emit stylesChanged();
    }
}

double LineStyleManager::getBaseLineThickness() const { return m_baseThickness; }

void LineStyleManager::setDashLength(double length)
{
    if (m_dashLength != length) {
        m_dashLength = length;
        emit stylesChanged();
    }
}

void LineStyleManager::setDashSpace(double space)
{
    if (m_dashSpace != space) {
        m_dashSpace = space;
        emit stylesChanged();
    }
}

// --- Работа с пользовательскими стилями ---

void LineStyleManager::addCustomStyle(int id, const QString& name, const QVector<qreal>& pattern)
{
    m_customStyles[id] = {name, pattern};
    emit stylesChanged();
}

void LineStyleManager::removeCustomStyle(int id)
{
    m_customStyles.remove(id);
    emit stylesChanged();
}

QMap<int, CustomLineStyle> LineStyleManager::getCustomStyles() const
{
    return m_customStyles;
}

int LineStyleManager::generateNewId() const
{
    int id = 1000; //Пользовательские ID начинаются с 1000
    while (m_customStyles.contains(id)) {
        id++;
    }
    return id;
}

QString LineStyleManager::getStyleName(int id) const
{
    //Сначала ищем в пользовательских
    if (m_customStyles.contains(id)) {
        return m_customStyles[id].name;
    }

    //Если не нашли, проверяем стандартные
    LineType type = static_cast<LineType>(id);
    switch(type) {
    case LineType::SolidMain: return "Сплошная основная";
    case LineType::SolidThin: return "Сплошная тонкая";
    case LineType::SolidWave: return "Сплошная волнистая";
    case LineType::Dashed:    return "Штриховая";
    case LineType::DashDotThick: return "Штрихпунктирная (толстая)";
    case LineType::DashDotThin:  return "Штрихпунктирная (тонкая)";
    case LineType::DashDotDot:   return "С двумя точками";
    case LineType::SolidKink:    return "С изломами";
    default: return "Линия";
    }
}

QVector<qreal> LineStyleManager::getPattern(int id) const
{
    QVector<qreal> basePattern;

    // 1. Получаем базовый паттерн (либо стандартный, либо пользовательский)
    if (m_customStyles.contains(id)) {
        basePattern = m_customStyles[id].pattern;
    } else {
        //Стандартные паттерны (заданы относительно s=1.0)
        double s = 1.0;
        LineType type = static_cast<LineType>(id);
        switch (type) {
        case LineType::Dashed:
            basePattern = {4.0 * s, 2.0 * s}; break;
        case LineType::DashDotThin:
        case LineType::DashDotThick:
            basePattern = {6.0 * s, 1.0 * s, 0.5 * s, 1.0 * s}; break;
        case LineType::DashDotDot:
            basePattern = {6.0 * s, 1.0 * s, 0.5 * s, 1.0 * s, 0.5 * s, 1.0 * s}; break;
        default:
            return QVector<qreal>(); // Сплошные линии
        }
    }

    // 2. Применяем глобальные коэффициенты масштабирования
    double dashScale = m_dashLength / DEFAULT_DASH_REF;
    double spaceScale = m_dashSpace / DEFAULT_SPACE_REF;

    QVector<qreal> scaledPattern;
    scaledPattern.reserve(basePattern.size());

    for (int i = 0; i < basePattern.size(); ++i) {
        if (i % 2 == 0) {
            // Четные индексы - "рисуемая" часть
            scaledPattern.append(basePattern[i] * dashScale);
        } else {
            // Нечетные индексы - "пробел"
            scaledPattern.append(basePattern[i] * spaceScale);
        }
    }

    return scaledPattern;
}

LineWeight LineStyleManager::getWeightForType(int typeId) const
{
    if (typeId >= 1000) return LineWeight::Thin;

    LineType type = static_cast<LineType>(typeId);
    switch (type) {
    case LineType::SolidMain:
    case LineType::DashDotThick:
        return LineWeight::Thick;
    default:
        return LineWeight::Thin;
    }
}

QPen LineStyleManager::getPen(int typeId, const QColor& color, bool isSelected) const
{
    Q_UNUSED(isSelected);

    QPen pen;
    pen.setColor(color);

    LineWeight weight = getWeightForType(typeId);
    double width = (weight == LineWeight::Thick) ? m_baseThickness : (m_baseThickness / 2.0);

    pen.setWidthF(width);
    pen.setCosmetic(true);
    pen.setCapStyle(Qt::FlatCap);

    QVector<qreal> dashes = getPattern(typeId);
    if (!dashes.isEmpty()) {
        pen.setDashPattern(dashes);
    } else {
        pen.setStyle(Qt::SolidLine);
    }

    return pen;
}

void LineStyleManager::drawLine(QPainter& painter, const QPointF& start, const QPointF& end,
                                int typeId, const QColor& color, bool isSelected) const
{
    // 1. ОТРИСОВКА КОНТУРА ВЫДЕЛЕНИЯ
    if (isSelected) {
        QPen highlightPen = getPen(typeId, color, false);
        highlightPen.setWidthF(highlightPen.widthF() + 8.0);
        QColor hColor = color;
        hColor.setAlpha(100);
        highlightPen.setColor(hColor);

        bool isSpecialHighlight = false;
        if (typeId < 1000) {
            LineType type = static_cast<LineType>(typeId);
            if (type == LineType::SolidWave) {
                highlightPen.setStyle(Qt::SolidLine);
                drawWaveLine(painter, start, end, highlightPen);
                isSpecialHighlight = true;
            }
            else if (type == LineType::SolidKink) {
                highlightPen.setStyle(Qt::SolidLine);
                drawZigzagLine(painter, start, end, highlightPen);
                isSpecialHighlight = true;
            }
        }
        if (!isSpecialHighlight) {
            painter.setPen(highlightPen);
            painter.drawLine(start, end);
        }
    }

    // 2. ОТРИСОВКА САМОЙ ЛИНИИ
    QPen standardPen = getPen(typeId, color, false);
    bool isSpecial = false;
    if (typeId < 1000) {
        LineType type = static_cast<LineType>(typeId);
        if (type == LineType::SolidWave) {
            standardPen.setStyle(Qt::SolidLine);
            drawWaveLine(painter, start, end, standardPen);
            isSpecial = true;
        }
        else if (type == LineType::SolidKink) {
            standardPen.setStyle(Qt::SolidLine);
            drawZigzagLine(painter, start, end, standardPen);
            isSpecial = true;
        }
    }
    if (!isSpecial) {
        painter.setPen(standardPen);
        painter.drawLine(start, end);
    }
}

void LineStyleManager::drawWaveLine(QPainter& painter, const QPointF& start, const QPointF& end, const QPen& pen) const
{
    painter.setPen(pen);
    QPainterPath path;
    path.moveTo(start);
    QLineF line(start, end);
    double length = line.length();
    double amplitude = 2.0;
    double period = 10.0;

    double dx = end.x() - start.x();
    double dy = end.y() - start.y();
    double nx = dx / length; // Нормализованный вектор
    double px = -dy / length; // Перпендикуляр
    double py = dx / length;

    for (double i = 0; i <= length; i += 2.0) {
        double t = i / length;
        double offset = amplitude * qSin(i * 2 * M_PI / period);
        double x = start.x() + dx * t + px * offset;
        double y = start.y() + dy * t + py * offset;
        path.lineTo(x, y);
    }
    path.lineTo(end);
    painter.drawPath(path);
}

void LineStyleManager::drawZigzagLine(QPainter& painter, const QPointF& start, const QPointF& end, const QPen& pen) const
{
    painter.setPen(pen);
    QPainterPath path;
    path.moveTo(start);
    QLineF line(start, end);
    double length = line.length();
    double amplitude = 3.0;
    double period = 15.0;

    double dx = end.x() - start.x();
    double dy = end.y() - start.y();
    double px = -dy / length; // Перпендикуляр
    double py = dx / length;

    for (double i = 0; i < length; i += period) {
        double t2 = (i + period / 4.0) / length; if (t2 > 1.0) t2 = 1.0;
        double topX = start.x() + dx * t2 + px * amplitude;
        double topY = start.y() + dy * t2 + py * amplitude;

        double t3 = (i + 3.0 * period / 4.0) / length; if (t3 > 1.0) t3 = 1.0;
        double botX = start.x() + dx * t3 - px * amplitude;
        double botY = start.y() + dy * t3 - py * amplitude;

        double t4 = (i + period) / length; if (t4 > 1.0) t4 = 1.0;
        double endPeriodX = start.x() + dx * t4;
        double endPeriodY = start.y() + dy * t4;

        path.lineTo(topX, topY);
        path.lineTo(botX, botY);
        path.lineTo(endPeriodX, endPeriodY);
    }
    path.lineTo(end);
    painter.drawPath(path);
}

void LineStyleManager::drawEllipse(QPainter& painter, const QPointF& center, double rx, double ry,
                                   int typeId, const QColor& color, bool isSelected) const
{
    // Определяем, является ли стиль "сложным" (волна или зигзаг)
    bool isWave = false;
    bool isZigzag = false;
    if (typeId < 1000) {
        LineType type = static_cast<LineType>(typeId);
        if (type == LineType::SolidWave) isWave = true;
        if (type == LineType::SolidKink) isZigzag = true;
    }

    // 1. ПОДЛОЖКА ПРИ ВЫДЕЛЕНИИ
    if (isSelected) {
        QPen highlightPen = getPen(typeId, color, false);
        highlightPen.setWidthF(highlightPen.widthF() + 8.0);
        QColor hColor = color;
        hColor.setAlpha(100);
        highlightPen.setColor(hColor);

        if (isWave || isZigzag) {
            highlightPen.setStyle(Qt::SolidLine); // Для кастомной отрисовки нужен Solid
            painter.setPen(highlightPen);
            painter.setBrush(Qt::NoBrush);
            // Рисуем аппроксимацию для подсветки
            // Можно упростить и рисовать просто эллипс для скорости,
            // но для точности лучше повторять форму.
            // Для простоты здесь рисуем обычный эллипс с толстой обводкой,
            // так как волна идет "вдоль" него.
            painter.drawEllipse(center, rx, ry);
        } else {
            painter.setPen(highlightPen);
            painter.setBrush(Qt::NoBrush);
            painter.drawEllipse(center, rx, ry);
        }
    }

    // 2. ОСНОВНАЯ ЛИНИЯ
    QPen standardPen = getPen(typeId, color, false);
    painter.setBrush(Qt::NoBrush);

    if (isWave) {
        standardPen.setStyle(Qt::SolidLine);
        painter.setPen(standardPen);

        // Рисуем волнистый эллипс
        QPainterPath path;
        double perimeter = 2 * M_PI * std::sqrt((rx*rx + ry*ry) / 2.0); // Приближенно
        double period = 10.0;
        double amplitude = 2.0;
        int steps = static_cast<int>(perimeter); // Шаг 1 пиксель
        if(steps < 10) steps = 10;

        for(int i=0; i<=steps; ++i) {
            double t = (double)i / steps; // 0..1
            double angle = t * 2 * M_PI;

            // Волна накладывается на радиус
            // Частота зависит от периметра и периода
            double waveOffset = amplitude * qSin((perimeter / period) * angle);

            double rCurrent = rx + waveOffset; // Упрощенно считаем rx=ry

            double x = center.x() + rCurrent * qCos(angle);
            double y = center.y() + rCurrent * qSin(angle);

            if (i==0) path.moveTo(x,y);
            else path.lineTo(x,y);
        }
        path.closeSubpath();
        painter.drawPath(path);

    }
    else if (isZigzag) {
        standardPen.setStyle(Qt::SolidLine);
        painter.setPen(standardPen);

        // Рисуем зигзаг эллипс (упрощенная реализация через ломаную)
        QPainterPath path;
        double perimeter = 2 * M_PI * std::sqrt((rx*rx + ry*ry) / 2.0);
        double period = 15.0;
        double amplitude = 3.0;

        int segments = std::round(perimeter / period);
        if (segments < 4) segments = 4;

        for(int i=0; i<segments; ++i) {
            double angleStart = (double)i / segments * 2 * M_PI;
            double angleEnd = (double)(i+1) / segments * 2 * M_PI;
            double angleMid = (angleStart + angleEnd) / 2.0;

            // Точки на окружности
            QPointF pStart(center.x() + rx * qCos(angleStart), center.y() + ry * qSin(angleStart));
            QPointF pEnd(center.x() + rx * qCos(angleEnd), center.y() + ry * qSin(angleEnd));

            // Точка "выброса" зигзага (наружу или внутрь)
            // Чередуем: четные наружу, нечетные внутрь? Или просто "пик" посередине?
            // Сделаем "пик" наружу
            double rMid = rx + amplitude;
            if (i%2 != 0) rMid = rx - amplitude;

            QPointF pMid(center.x() + rMid * qCos(angleMid), center.y() + rMid * qSin(angleMid));

            if (i==0) path.moveTo(pStart);
            path.lineTo(pMid);
            path.lineTo(pEnd);
        }
        path.closeSubpath();
        painter.drawPath(path);
    }
    else {
        // Стандартная отрисовка
        painter.setPen(standardPen);
        painter.drawEllipse(center, rx, ry);
    }
}
