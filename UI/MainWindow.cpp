#include "MainWindow.h"
#include <RockOutcrop.h>

#include <QRect>
#include <QGuiApplication>
#include <QScreen>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent)
{

    QRect rec = QGuiApplication::primaryScreen()->geometry();
    int height = this->height()<int(0.35*rec.height())?int(0.35*rec.height()):this->height();
    int width  = this->width()<int(0.35*rec.width())?int(0.35*rec.width()):this->width();
    this->resize(width, height);

    QWidget *theWidget = new RockOutcrop();
    this->setCentralWidget(theWidget);

    /*
    // margins of centralWidget
    centralWidget()->layout()->setContentsMargins(0, 0, 0, 0);
    statusBar()->hide();
    ui->mainToolBar->hide();
    */
}

MainWindow::~MainWindow()
{

}

