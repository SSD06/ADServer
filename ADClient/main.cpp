#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QFile file("./lightblue.css");
    if(file.open(QIODevice::ReadOnly))
    {
        a.setStyleSheet(file.readAll());
    }
//    else
//    {
//       qDebug()<<"没打开";
//    }
    MainWindow w;
    w.show();
    return a.exec();
}
