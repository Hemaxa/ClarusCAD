//BasePanelWidget — базовый класс для всех панелей в приложении

#pragma once

#include <QDockWidget>
#include <QWidget>

//QDockWidget - шаблон Qt для создания DockWidget (умеет перемещаться, стыковаться, прикпепляться к краям и пр.)
//QDockWidget - шаблон Qt для создания DockWidget (умеет перемещаться, стыковаться, прикпепляться к краям и пр.)
class BasePanelWidget : public QDockWidget
{
    Q_OBJECT

public:
    /**
     * @brief Конструктор панели.
     * @param title Заголовок панели.
     * @param parent Родительский виджет.
     */
    explicit BasePanelWidget(const QString& title, QWidget* parent);

    /**
     * @brief Виртуальный деструктор.
     */
    virtual ~BasePanelWidget() = default;

public slots:
    /**
     * @brief Слот обновления цветов иконок.
     * Вызывается при смене темы оформления.
     */
    virtual void updateColors();

protected:
    /**
     * @brief Получить доступ к холсту панели.
     * @return Указатель на QWidget холста.
     */
    QWidget* canvas() const;

private:
    QWidget* m_canvas; ///< Холст панели, на котором размещаются элементы.
};
