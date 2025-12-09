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

LineWeight LineStyleManager::getWeightForType(LineType type) const
{
    switch (type) {
    case LineType::SolidMain:
    case LineType::DashDotThick:
        return LineWeight::Thick;
    default:
        return LineWeight::Thin;
    }
}

QVector<qreal> LineStyleManager::getDashPattern(LineType type) const
{
    double s = 1.0; // Масштаб паттерна относительно толщины (можно вынести в m_dashScale)

    switch (type) {
    case LineType::Dashed:
        // Штриховая
        return QVector<qreal>{4.0 * s, 2.0 * s};
    case LineType::DashDotThin:
    case LineType::DashDotThick:
        // Штрихпунктирная
        return QVector<qreal>{6.0 * s, 1.0 * s, 0.5 * s, 1.0 * s};
    case LineType::DashDotDot:
        // Штрихпунктирная с двумя точками
        return QVector<qreal>{6.0 * s, 1.0 * s, 0.5 * s, 1.0 * s, 0.5 * s, 1.0 * s};
    default:
        return QVector<qreal>();
    }
}

QPen LineStyleManager::getPen(LineType type, const QColor& color, bool isSelected) const
{
    QPen pen;
    pen.setColor(isSelected ? QColor(0, 255, 127) : color);

    LineWeight weight = getWeightForType(type);
    double width = (weight == LineWeight::Thick) ? m_baseThickness : (m_baseThickness / 2.0);

    if (isSelected) width += 1.0;

    pen.setWidthF(width);
    pen.setCosmetic(true); // Важно для постоянной толщины при зуме
    pen.setCapStyle(Qt::FlatCap);

    QVector<qreal> dashes = getDashPattern(type);
    if (!dashes.isEmpty()) {
        pen.setDashPattern(dashes);
    } else {
        pen.setStyle(Qt::SolidLine);
    }

    return pen;
}

void LineStyleManager::drawLine(QPainter& painter, const QPointF& start, const QPointF& end,
                                LineType type, const QColor& color, bool isSelected) const
{
    // Получаем базовое перо (цвет, толщина)
    QPen pen = getPen(type, color, isSelected);

    // Для сложных типов линий используем специальные алгоритмы
    if (type == LineType::SolidWave) {
        // Волнистая всегда сплошная, паттерн пера сбрасываем, чтобы сама волна не была пунктирной
        pen.setStyle(Qt::SolidLine);
        drawWaveLine(painter, start, end, pen);
    }
    else if (type == LineType::SolidKink) {
        pen.setStyle(Qt::SolidLine);
        drawZigzagLine(painter, start, end, pen);
    }
    else {
        // Стандартная отрисовка (Qt умеет сам)
        painter.setPen(pen);
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

    // Параметры волны (можно вынести в настройки)
    double amplitude = 2.0; // Высота волны (в экранных пикселях, если Cosmetic=true, иначе скейлить)
    double period = 10.0;   // Длина одной волны

    // Вектор направления
    double dx = end.x() - start.x();
    double dy = end.y() - start.y();

    // Нормализованный вектор
    double nx = dx / length;
    double ny = dy / length;

    // Перпендикулярный вектор (-y, x)
    double px = -ny;
    double py = nx;

    // Генерируем синусоиду
    for (double i = 0; i <= length; i += 2.0) { // Шаг 2 пикселя для плавности
        double t = i / length;

        // Смещение по перпендикуляру
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
    // "Сплошная с изломами" - обычно это длинная прямая с зигзагом посередине (линия обрыва)
    // Но если нужна "Змейка" по всей длине, алгоритм похож на волну, но линейный.
    // Реализуем "Линию обрыва" (длинная линия с зигзагом по ГОСТ для длинных деталей).
    // Если подразумевалась "Змейка" (Zigzag) по всей длине - алгоритм будет другим.
    // Сделаем по всей длине для наглядности отличия от прямой.

    painter.setPen(pen);
    QPainterPath path;
    path.moveTo(start);

    QLineF line(start, end);
    double length = line.length();

    double amplitude = 3.0;
    double period = 15.0; // Шаг зуба

    // Вектор направления
    double dx = end.x() - start.x();
    double dy = end.y() - start.y();

    // Нормализованный
    double nx = dx / length;
    double ny = dy / length;

    // Перпендикуляр
    double px = -ny;
    double py = nx;

    // Рисуем треугольную волну
    for (double i = 0; i < length; i += period) {
        // Точка на оси
        double t1 = i / length;
        double centerX = start.x() + dx * t1;
        double centerY = start.y() + dy * t1;

        // Вершина (смещение +)
        double t2 = (i + period / 4.0) / length;
        if (t2 > 1.0) t2 = 1.0;
        double topX = start.x() + dx * t2 + px * amplitude;
        double topY = start.y() + dy * t2 + py * amplitude;

        // Низина (смещение -)
        double t3 = (i + 3.0 * period / 4.0) / length;
        if (t3 > 1.0) t3 = 1.0;
        double botX = start.x() + dx * t3 - px * amplitude;
        double botY = start.y() + dy * t3 - py * amplitude;

        // Конец периода
        double t4 = (i + period) / length;
        if (t4 > 1.0) t4 = 1.0;
        double endPeriodX = start.x() + dx * t4;
        double endPeriodY = start.y() + dy * t4;

        path.lineTo(topX, topY);
        path.lineTo(botX, botY);
        path.lineTo(endPeriodX, endPeriodY);
    }

    // Убеждаемся, что пришли точно в конец
    path.lineTo(end);

    painter.drawPath(path);
}
