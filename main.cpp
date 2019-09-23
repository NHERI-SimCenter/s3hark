#include "UI/MainWindow.h"
#include <QApplication>
#include <QChar>
#include <QFile>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    MainWindow m;
    m.setWindowTitle(QString("s") + QChar(0x00B3) + "hark - Site-specific Seismic Hazard Analysis & Research Kit" );
    m.show();


    QFile file(":/styleCommon/common_experimental.qss");

    if(file.open(QFile::ReadOnly)) {
      QString styleSheet = QLatin1String(file.readAll());

      a.setStyleSheet(styleSheet);
      file.close();
    }

    return a.exec();
}
