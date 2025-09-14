//BasePanelWidget — базовый класс для всех панелей в приложении

#pragma once

#include <QDockWidget>
#include <QWidget>

//QDockWidget - шаблон Qt для создания DockWidget (умеет перемещаться, стыковаться, прикпепляться к краям и пр.)
class BasePanelWidget : public QDockWidget
{
    Q_OBJECT

public:
    //explicit запрещает неявное преобразование типов для конструктора
    explicit BasePanelWidget(const QString& title, QWidget* parent = nullptr) : QDockWidget(title, parent)
    {
        //создание пустого универсального холста для окна
        m_canvas = new QWidget();

        //установка холста в качестве основного виджета QDockWidget
        setWidget(m_canvas);
    }

    //virtual означает, что деструктор создается автоматически из унаследованного класса
    virtual ~BasePanelWidget() = default;

protected:
    //доступ к холсту предоставлен только классам-наслденикам
    QWidget* canvas() const { return m_canvas; }

private:
    //указатель на холст
    QWidget* m_canvas;
};
