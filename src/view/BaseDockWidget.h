#pragma once

#include <QDockWidget>
#include <QWidget>

//QDockWidget - шаблон Qt для создания DockWidget (умеет перемещаться, стыковаться и пр.)
class BaseDockWidget : public QDockWidget
{
    Q_OBJECT

public:
    explicit BaseDockWidget(const QString& title, QWidget* parent = nullptr) : QDockWidget(title, parent)
    {
        //создание пустого универсального холста для окна
        m_canvas = new QWidget();

        //установка холста в качестве основного виджета QDockWidget
        setWidget(m_canvas);
    }

    //деструктор создается автоматически
    virtual ~BaseDockWidget() = default;

protected:
    //доступ к холсту предоставлен только классам-наслденикам
    QWidget* canvas() const { return m_canvas; }

private:
    //указатель на холст
    QWidget* m_canvas;
};
