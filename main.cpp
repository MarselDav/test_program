#include "mainwindow.h"

#include <QApplication>

#include <QDirIterator>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.resize(500, 700);
    w.show();

    return a.exec();
}
