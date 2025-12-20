//BaseDrawingTool — базовый класс для всех отрисовщиков в приложении

#pragma once

class QPainter;
class BasePrimitive;

class BaseDrawingTool
{
public:
    /**
     * @brief Виртуальный деструктор.
     * Гарантирует корректное удаление наследников.
     */
    virtual ~BaseDrawingTool() = default;

    /**
     * @brief Метод отрисовки примитива.
     * @param painter Ссылка на QPainter для рисования.
     * @param primitive Указатель на объект данных, который нужно нарисовать.
     * @param isSelected Флаг выделения. Если true, объект рисуется с эффектом выделения.
     * 
     * Это чистый виртуальный метод (абстрактный), который обязан быть реализован в наследниках.
     */
    virtual void draw(QPainter& painter, BasePrimitive* primitive, bool isSelected = false) const = 0;
};
