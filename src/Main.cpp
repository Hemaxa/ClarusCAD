#include "MainWindow.h"

#include <QApplication>
#include <QFile>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    //загрузка файла стилей из ресурсов
    QFile styleFile(":/themes/dark_green.qss");
    if (styleFile.open(QFile::ReadOnly)) {
        QString styleSheet = QLatin1String(styleFile.readAll());
        a.setStyleSheet(styleSheet);
        styleFile.close();
    }

    //создание объекта главного окна
    MainWindow w;
    w.show();
    return a.exec();
}
