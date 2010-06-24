#include <QtGui>
#include "mainwindow.h"

int main(int argc, char * argv[])
{
    QApplication app(argc, argv);
    MainWindow browser;
    browser.show();
    return app.exec();
}
