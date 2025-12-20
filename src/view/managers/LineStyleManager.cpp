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

double LineStyleManager::getDashLength() const { return m_dashLength; }

void LineStyleManager::setDashSpace(double space)
{
    if (m_dashSpace != space) {
        m_dashSpace = space;
        emit stylesChanged();
    }
}

double LineStyleManager::getDashSpace() const { return m_dashSpace; }

// --- Параметры волнистой линии ---

void LineStyleManager::setWaveAmplitude(double val)
{
    if (val > 0 && m_waveAmplitude != val) {
        m_waveAmplitude = val;
        emit stylesChanged();
    }
}

double LineStyleManager::getWaveAmplitude() const { return m_waveAmplitude; }

void LineStyleManager::setWavePeriod(double val)
{
    if (val > 0 && m_wavePeriod != val) {
        m_wavePeriod = val;
        emit stylesChanged();
    }
}

double LineStyleManager::getWavePeriod() const { return m_wavePeriod; }

// --- Параметры линии с изломами ---

void LineStyleManager::setKinkAmplitude(double val)
{
    if (val > 0 && m_kinkAmplitude != val) {
        m_kinkAmplitude = val;
        emit stylesChanged();
    }
}

double LineStyleManager::getKinkAmplitude() const { return m_kinkAmplitude; }

void LineStyleManager::setKinkLength(double val)
{
    if (val > 0 && m_kinkLength != val) {
        m_kinkLength = val;
        emit stylesChanged();
    }
}

double LineStyleManager::getKinkLength() const { return m_kinkLength; }

void LineStyleManager::setKinkStraight(double val)
{
    if (val > 0 && m_kinkStraight != val) {
        m_kinkStraight = val;
        emit stylesChanged();
    }
}

double LineStyleManager::getKinkStraight() const { return m_kinkStraight; }

// --- Работа с пользовательскими стилями ---

void LineStyleManager::addCustomStyle(int id, const QString& name, const QVector<qreal>& pattern, double thickness)
{
    m_customStyles[id] = {name, pattern, thickness};
    emit stylesChanged();
}

void LineStyleManager::updateCustomStyle(int id, const QString& name, const QVector<qreal>& pattern, double thickness)
{
    if (m_customStyles.contains(id)) {
        m_customStyles[id] = {name, pattern, thickness};
        emit stylesChanged();
    }
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
    if (length < 1.0) { painter.drawLine(start, end); return; }

    // Используем настраиваемые параметры
    double amplitude = m_waveAmplitude;
    double period = m_wavePeriod;

    double dx = end.x() - start.x();
    double dy = end.y() - start.y();
    double px = -dy / length; // Перпендикуляр
    double py = dx / length;

    double step = qMax(1.0, period / 10.0); // Шаг для гладкости
    for (double i = 0; i <= length; i += step) {
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
    // Линия с изломами по ГОСТ: вниз - вверх - прямой участок - повтор
    painter.setPen(pen);
    QPainterPath path;
    path.moveTo(start);
    QLineF line(start, end);
    double length = line.length();
    if (length < 1.0) { painter.drawLine(start, end); return; }

    // Используем настраиваемые параметры
    double amplitude = m_kinkAmplitude;
    double kinkLen = m_kinkLength;
    double straightLen = m_kinkStraight;
    double period = 2 * kinkLen + straightLen; // Полный цикл: вниз + вверх + прямой

    double dx = end.x() - start.x();
    double dy = end.y() - start.y();
    double px = -dy / length; // Перпендикуляр (нормаль)
    double py = dx / length;

    double currentPos = 0;
    while (currentPos < length) {
        // 1. Точка вниз (ниже линии)
        double t1 = (currentPos + kinkLen / 2.0) / length;
        if (t1 > 1.0) t1 = 1.0;
        double downX = start.x() + dx * t1 - px * amplitude;
        double downY = start.y() + dy * t1 - py * amplitude;
        path.lineTo(downX, downY);

        // 2. Точка вверх (выше линии)
        double t2 = (currentPos + kinkLen) / length;
        if (t2 > 1.0) { path.lineTo(end); break; }
        double upX = start.x() + dx * t2 + px * amplitude;
        double upY = start.y() + dy * t2 + py * amplitude;
        path.lineTo(upX, upY);

        // 3. Возврат на линию и прямой участок
        double t3 = (currentPos + kinkLen + kinkLen / 2.0) / length;
        if (t3 > 1.0) t3 = 1.0;
        double backX = start.x() + dx * t3;
        double backY = start.y() + dy * t3;
        path.lineTo(backX, backY);

        // 4. Конец прямого участка
        currentPos += period;
        double t4 = currentPos / length;
        if (t4 > 1.0) t4 = 1.0;
        double endX = start.x() + dx * t4;
        double endY = start.y() + dy * t4;
        path.lineTo(endX, endY);
    }
    path.lineTo(end);
    painter.drawPath(path);
}

double LineStyleManager::drawWaveLineWithPhase(QPainter& painter, const QPointF& start, const QPointF& end, 
                                                const QPen& pen, double startPhase) const
{
    painter.setPen(pen);
    QPainterPath path;
    path.moveTo(start);
    QLineF line(start, end);
    double length = line.length();
    if (length < 1.0) { 
        painter.drawLine(start, end); 
        return startPhase + length; 
    }

    double amplitude = m_waveAmplitude;
    double period = m_wavePeriod;

    double dx = end.x() - start.x();
    double dy = end.y() - start.y();
    double px = -dy / length; // Perpendicular
    double py = dx / length;

    double step = qMax(1.0, period / 10.0);
    for (double i = 0; i <= length; i += step) {
        double t = i / length;
        // Use startPhase to continue wave pattern from previous segment
        double offset = amplitude * qSin((startPhase + i) * 2 * M_PI / period);
        double x = start.x() + dx * t + px * offset;
        double y = start.y() + dy * t + py * offset;
        path.lineTo(x, y);
    }
    path.lineTo(end);
    painter.drawPath(path);
    
    return startPhase + length; // Return accumulated distance
}

double LineStyleManager::drawZigzagLineWithPhase(QPainter& painter, const QPointF& start, const QPointF& end, 
                                                  const QPen& pen, double startPhase) const
{
    painter.setPen(pen);
    QPainterPath path;
    path.moveTo(start);
    QLineF line(start, end);
    double length = line.length();
    if (length < 1.0) { 
        painter.drawLine(start, end); 
        return startPhase + length; 
    }

    double amplitude = m_kinkAmplitude;
    double kinkLen = m_kinkLength;
    double straightLen = m_kinkStraight;
    double period = 2 * kinkLen + straightLen;

    double dx = end.x() - start.x();
    double dy = end.y() - start.y();
    double px = -dy / length;
    double py = dx / length;

    // Start from where we left off in the pattern
    double currentPos = std::fmod(startPhase, period);
    double localPos = 0;
    
    while (localPos < length) {
        double phaseInPeriod = std::fmod(currentPos, period);
        
        if (phaseInPeriod < kinkLen / 2.0) {
            // Going down
            double remaining = kinkLen / 2.0 - phaseInPeriod;
            double stepLen = std::min(remaining, length - localPos);
            double progress = (phaseInPeriod + stepLen) / (kinkLen / 2.0);
            double t = (localPos + stepLen) / length;
            double offsetY = -amplitude * progress;
            path.lineTo(start.x() + dx * t + px * offsetY, start.y() + dy * t + py * offsetY);
            localPos += stepLen;
            currentPos += stepLen;
        }
        else if (phaseInPeriod < kinkLen) {
            // Going up
            double remaining = kinkLen - phaseInPeriod;
            double stepLen = std::min(remaining, length - localPos);
            double progress = (phaseInPeriod - kinkLen / 2.0 + stepLen) / (kinkLen / 2.0);
            double t = (localPos + stepLen) / length;
            double offsetY = -amplitude + 2 * amplitude * progress;
            path.lineTo(start.x() + dx * t + px * offsetY, start.y() + dy * t + py * offsetY);
            localPos += stepLen;
            currentPos += stepLen;
        }
        else if (phaseInPeriod < kinkLen + kinkLen / 2.0) {
            // Returning to center
            double remaining = kinkLen + kinkLen / 2.0 - phaseInPeriod;
            double stepLen = std::min(remaining, length - localPos);
            double progress = (phaseInPeriod - kinkLen + stepLen) / (kinkLen / 2.0);
            double t = (localPos + stepLen) / length;
            double offsetY = amplitude * (1.0 - progress);
            path.lineTo(start.x() + dx * t + px * offsetY, start.y() + dy * t + py * offsetY);
            localPos += stepLen;
            currentPos += stepLen;
        }
        else {
            // Straight section
            double remaining = period - phaseInPeriod;
            double stepLen = std::min(remaining, length - localPos);
            double t = (localPos + stepLen) / length;
            path.lineTo(start.x() + dx * t, start.y() + dy * t);
            localPos += stepLen;
            currentPos += stepLen;
        }
    }
    path.lineTo(end);
    painter.drawPath(path);
    
    return startPhase + length;
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

    // Если обычная линия, используем стандартную отрисовку
    if (!isWave && !isZigzag) {
        if (isSelected) {
            QPen highlightPen = getPen(typeId, color, false);
            highlightPen.setWidthF(highlightPen.widthF() + 8.0);
            QColor hColor = color;
            hColor.setAlpha(100);
            highlightPen.setColor(hColor);
            painter.setPen(highlightPen);
            painter.setBrush(Qt::NoBrush);
            painter.drawEllipse(center, rx, ry);
        }

        QPen standardPen = getPen(typeId, color, false);
        painter.setPen(standardPen);
        painter.setBrush(Qt::NoBrush);
        painter.drawEllipse(center, rx, ry);
        return;
    }

    // ДЛЯ ВОЛНЫ И ЗИГЗАГА ИСПОЛЬЗУЕМ РАДИАЛЬНЫЙ АЛГОРИТМ
    // Генерируем путь, модулируя радиус в зависимости от угла

    double perimeter = 2 * M_PI * std::sqrt((rx*rx + ry*ry) / 2.0);
    double amplitude = isWave ? m_waveAmplitude : m_kinkAmplitude;
    double period = isWave ? m_wavePeriod : (2 * m_kinkLength + m_kinkStraight);
    
    // Количество циклов волны на весь периметр
    double cycles = perimeter / period;
    // Округляем до целого, чтобы волна замкнулась идеально
    cycles = std::round(cycles);
    if (cycles < 1.0) cycles = 1.0;
    
    // Пересчитываем реальный период (угловой)
    // 2*PI = cycles * anglePerPeriod
    double anglePerPeriod = 2 * M_PI / cycles;
    
    QPainterPath path;
    
    // Шаг дискретизации - достаточно малый для плавности
    // Например, 40 точек на один период волны
    int stepsPerPeriod = 40;
    int totalSteps = static_cast<int>(cycles * stepsPerPeriod);
    
    for (int i = 0; i <= totalSteps; ++i) {
        double t = static_cast<double>(i) / totalSteps; // от 0 до 1
        double angle = t * 2 * M_PI; // от 0 до 2PI
        
        // Вычисляем смещение радиуса
        double offset = 0.0;
        
        if (isWave) {
            // Sinusoidal wave: offset = A * sin(angle * cycles)
            // Но angle идет от 0 до 2PI, значит аргумент синуса от 0 до cycles*2PI
            offset = amplitude * std::sin(angle * cycles);
        } else {
            // Zigzag (triangle wave)
            // Фаза внутри одного цикла (от 0 до 1)
            double phase = t * cycles; 
            phase -= std::floor(phase); // frac part
            
            // Triangle shape: 0 -> A -> -A -> 0 (или подобное)
            // Реализуем симметричный зигзаг: 0 -> 1 -> 0 -> -1 -> 0
            // phase 0..0.25: 0 to A
            // phase 0.25..0.75: A to -A
            // phase 0.75..1.0: -A to 0
            
            if (phase < 0.25) {
                offset = amplitude * (phase / 0.25);
            } else if (phase < 0.75) {
                offset = amplitude * (1.0 - (phase - 0.25) / 0.25); // A -> -A
            } else {
                offset = -amplitude * (1.0 - (phase - 0.75) / 0.25); // -A -> 0
            }
        }
        
        // Модулируем радиус
        double rX = rx + offset;
        double rY = ry + offset;
        
        double x = center.x() + rX * std::cos(angle);
        double y = center.y() + rY * std::sin(angle);
        
        if (i == 0) path.moveTo(x, y);
        else path.lineTo(x, y);
    }
    path.closeSubpath();
    
    // ОТРИСОВКА
    painter.setBrush(Qt::NoBrush);
    
    if (isSelected) {
        QPen highlightPen = getPen(typeId, color, false);
        highlightPen.setWidthF(highlightPen.widthF() + 8.0);
        highlightPen.setStyle(Qt::SolidLine); // Рисуем саму геометрию сплошной толстой линией
        QColor hColor = color;
        hColor.setAlpha(100);
        highlightPen.setColor(hColor);
        painter.setPen(highlightPen);
        painter.drawPath(path);
    }
    
    QPen standardPen = getPen(typeId, color, false);
    standardPen.setStyle(Qt::SolidLine);
    painter.setPen(standardPen);
    painter.drawPath(path);
}

void LineStyleManager::drawArc(QPainter& painter, const QPointF& center, double radius,
                                double startAngle, double spanAngle,
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

    if (!isWave && !isZigzag) {
        // Standard arc drawing
        QRectF rect(center.x() - radius, center.y() - radius, radius * 2, radius * 2);
        int startAngle16 = static_cast<int>(startAngle * 16);
        int spanAngle16 = static_cast<int>(spanAngle * 16);

        QPen pen = getPen(typeId, color, false);

        if (isSelected) {
            QPen hPen = pen;
            hPen.setWidthF(pen.widthF() + 8.0);
            QColor hColor = color;
            hColor.setAlpha(100);
            hPen.setColor(hColor);
            painter.setPen(hPen);
            painter.setBrush(Qt::NoBrush);
            painter.drawArc(rect, startAngle16, spanAngle16);
        }

        painter.setPen(pen);
        painter.setBrush(Qt::NoBrush);
        painter.drawArc(rect, startAngle16, spanAngle16);
        return;
    }

    // ДЛЯ ВОЛНЫ И ЗИГЗАГА НА ДУГЕ - РАДИАЛЬНЫЙ АЛГОРИТМ
    
    double startRad = startAngle * M_PI / 180.0;
    double spanRad = spanAngle * M_PI / 180.0;
    
    // Qt углы: положительные = против часовой. 
    // Но экран Y вниз. Обычно в Qt: 0 = 3 часа, + = против часовой.
    // Отрисовка через cos/sin работает стандартно: x = cx + r*cos(a), y = cy - r*sin(a) (для Y вверх)
    // Но здесь у нас QPointF и экранные координаты.
    // x = cx + r*cos(a), y = cy + r*sin(a) — это по часовой, если a растет.
    // drawArc в Qt принимает startAngle в градусах (CCW on screen? No, CCW in mathematical sense usually leads to CW on Y-down screen unless transform is applied).
    // Стандартное поведение QPainter::drawArc: 0 градусов - 3 часа, положительные - против часовой стрелки.
    // Поскольку у нас Y направлен вниз, "против часовой" на экране выглядит "по часовой" математически?
    // Нет, Qt абстрагирует это. angle=90 - это 12 часов.
    // x = cos(-angle), y = sin(-angle) для экранных координат?
    // Проще: используем отрицательный угол в sin, так как Y инвертирован?
    // Давайте следовать той же логике, что и в drawEllipse выше: x = cos(a), y = sin(a). 
    // Если выше работало (а там полный круг), то и тут сработает, нужно только правильно интерпретировать углы.
    // QPainter::drawArc argument is in 1/16th degrees. Positive is counter-clockwise.
    // Значит angle идет: 0 (Right), 90 (Top/Up), 180 (Left), 270 (Bottom).
    // Формула: x = cx + r * cos(-angleRad), y = cy + r * sin(-angleRad) ??
    // Учитывая Y вниз: 
    // 0 deg: (1, 0) -> Right. Correct.
    // 90 deg: (0, -1) -> Top. Correct.
    // Значит формула: x = cx + r * cos(-angle), y = cy + r * sin(-angle). 
    
    // Однако, выше в drawEllipse я использовал просто cos(angle), sin(angle).
    // При angle 0..2PI это рисует круг по часовой (из-за Y вниз). 
    // 0 -> (1,0) Right.
    // PI/2 -> (0,1) Bottom!
    // Значит мой drawEllipse рисует "математически по часовой" (визуально на экране).
    
    // Для дуги нужно совпадать с drawArc. DrawArc (Qt) рисует CCW.
    // Значит нам нужно итерироваться от startAngle до startAngle+spanAngle.
    // И использовать формулу для CCW в экранных координатах (Y-down).
    // X = cx + r * cos(-theta)
    // Y = cy + r * sin(-theta) 
    
    double arcLen = std::abs(radius * spanRad);
    double amplitude = isWave ? m_waveAmplitude : m_kinkAmplitude;
    double period = isWave ? m_wavePeriod : (2 * m_kinkLength + m_kinkStraight);
    
    double cycles = arcLen / period;
    // Здесь не обязательно округлять до целого, так как дуга не замкнута
    
    double totalAngle = std::abs(spanRad);
    int totalSteps = std::max(10, static_cast<int>(cycles * 40)); 
    
    QPainterPath path;
    
    for (int i = 0; i <= totalSteps; ++i) {
        double t = static_cast<double>(i) / totalSteps;
        
        // Текущий угол в радианах (математический CCW)
        double currentAngleRad = startRad + t * spanRad;
        
        // Вычисляем смещение
        double offset = 0.0;
        
        // Линейная фаза вдоль дуги
        double phase = t * cycles;
        
        if (isWave) {
             offset = amplitude * std::sin(phase * 2 * M_PI);
        } else {
             // Zigzag logic
             double p = phase - std::floor(phase);
             if (p < 0.25) offset = amplitude * (p / 0.25);
             else if (p < 0.75) offset = amplitude * (1.0 - (p - 0.25) / 0.25);
             else offset = -amplitude * (1.0 - (p - 0.75) / 0.25);
        }
        
        double r = radius + offset;
        
        // Преобразуем полярные CCW координаты в экранные (Y-down)
        // Angle 0 -> Right (1,0)
        // Angle 90 -> Top (0,-1)
        double x = center.x() + r * std::cos(-currentAngleRad);
        double y = center.y() + r * std::sin(-currentAngleRad);
        
        if (i == 0) path.moveTo(x, y);
        else path.lineTo(x, y);
    }
    
    painter.setBrush(Qt::NoBrush);

    if (isSelected) {
        QPen highlightPen = getPen(typeId, color, false);
        highlightPen.setWidthF(highlightPen.widthF() + 8.0);
        highlightPen.setStyle(Qt::SolidLine);
        QColor hColor = color;
        hColor.setAlpha(100);
        highlightPen.setColor(hColor);
        painter.setPen(highlightPen);
        painter.drawPath(path);
    }
    
    QPen standardPen = getPen(typeId, color, false);
    standardPen.setStyle(Qt::SolidLine);
    painter.setPen(standardPen);
    painter.drawPath(path);
}

void LineStyleManager::drawPath(QPainter& painter, const QPainterPath& path,
                                 int typeId, const QColor& color, bool isSelected) const
{
    // Determine if special line type
    bool isWave = false;
    bool isZigzag = false;
    if (typeId < 1000) {
        LineType type = static_cast<LineType>(typeId);
        if (type == LineType::SolidWave) isWave = true;
        if (type == LineType::SolidKink) isZigzag = true;
    }

    if (!isWave && !isZigzag) {
        // Standard path drawing
        QPen pen = getPen(typeId, color, false);

        if (isSelected) {
            QPen hPen = pen;
            hPen.setWidthF(pen.widthF() + 8.0);
            QColor hColor = color;
            hColor.setAlpha(100);
            hPen.setColor(hColor);
            painter.setPen(hPen);
            painter.setBrush(Qt::NoBrush);
            painter.drawPath(path);
        }

        painter.setPen(pen);
        painter.setBrush(Qt::NoBrush);
        painter.drawPath(path);
        return;
    }

    // For wave/zigzag, convert path to polygon and draw segments with continuous phase
    // Flatten curves into line segments
    QList<QPolygonF> polygons = path.toSubpathPolygons();
    
    QPen pen = getPen(typeId, color, false);
    pen.setStyle(Qt::SolidLine);

    // Draw selection highlight with phase tracking
    if (isSelected) {
        QPen hPen = pen;
        hPen.setWidthF(pen.widthF() + 8.0);
        QColor hColor = color;
        hColor.setAlpha(100);
        hPen.setColor(hColor);
        
        for (const QPolygonF& poly : polygons) {
            double phase = 0.0;
            for (int i = 0; i < poly.size() - 1; ++i) {
                QPointF p1 = poly[i];
                QPointF p2 = poly[i + 1];
                
                if (isWave) phase = drawWaveLineWithPhase(painter, p1, p2, hPen, phase);
                else phase = drawZigzagLineWithPhase(painter, p1, p2, hPen, phase);
            }
        }
    }

    // Draw main line with phase tracking
    for (const QPolygonF& poly : polygons) {
        double phase = 0.0;
        for (int i = 0; i < poly.size() - 1; ++i) {
            QPointF p1 = poly[i];
            QPointF p2 = poly[i + 1];
            
            if (isWave) phase = drawWaveLineWithPhase(painter, p1, p2, pen, phase);
            else phase = drawZigzagLineWithPhase(painter, p1, p2, pen, phase);
        }
    }
}
