#include "UI/MainWindow.h"
#include <QApplication>


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    MainWindow m;
    m.setWindowTitle("Site Response Tool");
    m.show();

    return a.exec();
}
