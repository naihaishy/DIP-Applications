#include "mainwindow.h"
#include <QApplication>
#include <QTranslator>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // 翻译
    QTranslator translator;
    translator.load(":/language/ImageProcess_CN.qm");
    a.installTranslator(&translator);

    MainWindow w;
    w.show();

    return a.exec();
}
