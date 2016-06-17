#include "mainwindow.h"
#include <QApplication>
#include <QTextCodec>

int main(int argc, char *argv[])
{
    QCoreApplication::addLibraryPath("./plugins");
    
    QApplication a(argc, argv);

    MainWindow w;
    w.show();

    return a.exec();
}
