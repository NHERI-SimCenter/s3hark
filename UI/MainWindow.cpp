#include "MainWindow.h"
#include <RockOutcrop1D.h>

#include <QRect>
#include <QGuiApplication>
#include <QScreen>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent)
{

    QRect rec = QGuiApplication::primaryScreen()->geometry();
        int height = this->height()<int(0.85*rec.height())?int(0.85*rec.height()):this->height();
        int width  = this->width()<int(0.85*rec.width())?int(0.85*rec.width()):this->width();
    this->resize(width, height);

    QWidget *theWidget = new RockOutcrop1D();
    this->setCentralWidget(theWidget);

    // set the global stylesheet
    QFile file(":/resources/styles/stylesheet.css");
    if(file.open(QFile::ReadOnly)) {
      QString styleSheet = QLatin1String(file.readAll());
      this->setStyleSheet(styleSheet);
    }
}

MainWindow::~MainWindow()
{

}

