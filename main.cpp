#include "UI/MainWindow.h"
#include <QApplication>
#include <QChar>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    MainWindow m;
    m.setWindowTitle(QString("s") + QChar(0x00B3) + "hark - Site-specific Seismic Hazard Analysis & Research Kit" );
    m.show();

    return a.exec();
}
