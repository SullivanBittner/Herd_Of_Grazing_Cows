#include "herd_of_grazing_cows.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Herd_of_Grazing_Cows w;
    w.show();
    return a.exec();
}
