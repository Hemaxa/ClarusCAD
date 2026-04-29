//BaseCreationTool — базовый класс для всех инструментов в приложении

#pragma once

#include "EnumManager.h"
#include "../view/managers/ThemeManager.h"

#include <QObject>
#include <QColor>

class BasePrimitive;
class Scene;
class ViewportPanelWidget;
class QMouseEvent;
class QKeyEvent;
class QPainter;

class BaseCreationTool : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief Конструктор.
     * @param parent Родительский объект.
     */
    explicit BaseCreationTool(QObject* parent = nullptr) : QObject(parent) {}

    /**
     * @brief Виртуальный деструктор.
     */
    virtual ~BaseCreationTool() = default;

    /**
     * @brief Обработка нажатия кнопки мыши.
     * @param event Событие мыши.
     * @param scene Указатель на сцену.
     * @param viewport Указатель на виджет просмотра.
     */
    virtual void onMousePress(QMouseEvent* event, Scene* scene, ViewportPanelWidget* viewport) = 0;

    /**
     * @brief Обработка перемещения мыши.
     * @param event Событие мыши.
     * @param scene Указатель на сцену.
     * @param viewport Указатель на виджет просмотра.
     */
    virtual void onMouseMove(QMouseEvent* event, Scene* scene, ViewportPanelWidget* viewport) = 0;

    /**
     * @brief Обработка отпускания кнопки мыши.
     * @param event Событие мыши.
     * @param scene Указатель на сцену.
     * @param viewport Указатель на виджет просмотра.
     */
    virtual void onMouseRelease(QMouseEvent* event, Scene* scene, ViewportPanelWidget* viewport) = 0;

    /**
     * @brief Сброс состояния инструмента.
     * Вызывается при отмене операции или переключении инструментов.
     */
    virtual void reset() {}

    /**
     * @brief Обработка нажатия клавиши клавиатуры.
     * @param event Событие клавиатуры.
     * @param scene Указатель на сцену.
     * @param viewport Указатель на виджет просмотра.
     */
    virtual void onKeyPress(QKeyEvent* event, Scene* scene, ViewportPanelWidget* viewport) {
        Q_UNUSED(event); Q_UNUSED(scene); Q_UNUSED(viewport);
    }

    /**
     * @brief Отрисовка временной геометрии инструмента (предпросмотр).
     * @param painter Объект QPainter для рисования.
     */
    virtual void onPaint(QPainter& painter) { Q_UNUSED(painter); }

    /**
     * @brief Установить текущий цвет для создаваемых объектов.
     * @param color Цвет.
     */
    virtual void setColor(const QColor& color) { Q_UNUSED(color); }

    /**
     * @brief Установить текущий тип линии для создаваемых объектов.
     * @param type Тип линии.
     */
    virtual void setLineType(LineType type) { Q_UNUSED(type); }

    /**
     * @brief Получить текущий цвет.
     */
    virtual QColor getColor() const { return ThemeManager::instance().getColor("drawingColor"); }

    /**
     * @brief Получить текущий тип линии.
     */
    virtual LineType getLineType() const { return LineType::SolidMain; }
};
