#include "driver_loader.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    DriverLoader w;
    w.show();
    return a.exec();
}
