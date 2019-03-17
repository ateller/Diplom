#include "control.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    control s;
    s.show();

    return a.exec();
}
