#include "servergui.h"



int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    ServerGui w;
    w.show();
    return a.exec();
}
