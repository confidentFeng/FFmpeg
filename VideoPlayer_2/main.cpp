#include "OpenGlWidget.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    OpenGlWidget w;
    w.show();

    return a.exec();
}
