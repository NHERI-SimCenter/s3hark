#include "MainWindow.h"
#include <RockOutcrop.h>

#include <QRect>
#include <QGuiApplication>
#include <QScreen>
#include <QVBoxLayout>
#include <HeaderWidget.h>
#include <FooterWidget.h>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QDesktopServices>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent)
{

    QRect rec = QGuiApplication::primaryScreen()->geometry();
    int height = this->height()<int(0.35*rec.height())?int(0.35*rec.height()):this->height();
    int width  = this->width()<int(0.35*rec.width())?int(0.35*rec.width()):this->width();
    this->resize(width, height);

    QWidget *centralWidget = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout();
    centralWidget->setLayout(layout);

    //
    // add SimCenter Header
    //

    //QString appName(QString("s") + QChar(0x00B3) + "hark");
    QString appName = "<P><b><i><FONT COLOR='#000000' FONT SIZE = 4>";
    appName.append(QString("S") + QChar(0x00B3) + "HARK    &nbsp;&nbsp;&nbsp;&nbsp;🦈");
    appName.append("</i></b></P></br>");
    HeaderWidget *header = new HeaderWidget();
    header->setHeadingText(appName);
    layout->addWidget(header);

    theRockOutcropWidget = new RockOutcrop();
    //    this->setCentralWidget(theWidget);
    layout->addWidget(theRockOutcropWidget);

    //
    // add SimCenter footer
    //

    FooterWidget *footer = new FooterWidget();
    layout->addWidget(footer);

    this->setCentralWidget(centralWidget);

    this->createActions();
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


void MainWindow::createActions() {
    QMenu *fileMenu = menuBar()->addMenu(tr("&File"));

    QAction *openAction = new QAction(tr("&Open"), this);
    openAction->setShortcuts(QKeySequence::Open);
    openAction->setStatusTip(tr("Open an existing file"));
    connect(openAction, &QAction::triggered, this, &MainWindow::open);
    fileMenu->addAction(openAction);
    //fileToolBar->addAction(openAction);


    QAction *saveAction = new QAction(tr("&Save"), this);
    saveAction->setShortcuts(QKeySequence::Save);
    saveAction->setStatusTip(tr("Save the document to disk"));
    connect(saveAction, &QAction::triggered, this, &MainWindow::save);
    fileMenu->addAction(saveAction);

    QAction *saveAsAction = new QAction(tr("&Save As"), this);
    saveAsAction->setStatusTip(tr("Save the document with new filename to disk"));
    connect(saveAsAction, &QAction::triggered, this, &MainWindow::saveAs);
    fileMenu->addAction(saveAsAction);

    /*
    thePreferences = SimCenterPreferences::getInstance(this);
    QAction *preferenceAction = new QAction(tr("&Preferences"), this);
    preferenceAction->setStatusTip(tr("Set application preferences"));
    connect(preferenceAction, &QAction::triggered, this, &MainWindow::preferences);
    fileMenu->addAction(preferenceAction);
    */

    // strangely, this does not appear in menu (at least on a mac)!! ..
    // does Qt not allow as in tool menu by default?
    // check for yourself by changing Quit to drivel and it works
    QAction *exitAction = new QAction(tr("&Quit"), this);
    connect(exitAction, SIGNAL(triggered()), qApp, SLOT(quit()));
    exitAction->setStatusTip(tr("Exit the application"));
    fileMenu->addAction(exitAction);

    QMenu *helpMenu = menuBar()->addMenu(tr("&Help"));
    QAction *versionAct = helpMenu->addAction(tr("&Version"), this, &MainWindow::version);
    QAction *aboutAct = helpMenu->addAction(tr("&About"), this, &MainWindow::about);
    QAction *manualAct = helpMenu->addAction(tr("&Manual"), this, &MainWindow::manual);
    QAction *submitAct = helpMenu->addAction(tr("&Provide Feedback"), this, &MainWindow::submitFeedback);
    QAction *submitFeature = helpMenu->addAction(tr("&Submit Feature Request"), this, &MainWindow::submitFeatureRequest);
    QAction *citeAct = helpMenu->addAction(tr("&How to Cite"), this, &MainWindow::cite);
    QAction *copyrightAct = helpMenu->addAction(tr("&License"), this, &MainWindow::copyright);
}


bool MainWindow::save()
{
    if (currentFile.isEmpty()) {
        return saveAs();
    } else {
        return saveFile(currentFile);
    }
}

bool MainWindow::saveAs()
{
    //
    // get filename
    //

    QFileDialog dialog(this);
    dialog.setWindowModality(Qt::WindowModal);
    dialog.setAcceptMode(QFileDialog::AcceptSave);
    if (dialog.exec() != QDialog::Accepted)
        return false;

    // and save the file
    return saveFile(dialog.selectedFiles().first());
}

void MainWindow::open()
{
    QString fileName = QFileDialog::getOpenFileName(this);
    if (!fileName.isEmpty())
        loadFile(fileName);
}


void MainWindow::openFile(QString fileName)
{
    if (!fileName.isEmpty())
        loadFile(fileName);
}

void MainWindow::newFile()
{
    // clear old
   // inputWidget->clear();

    // set currentFile blank
    setCurrentFile(QString());
}


void MainWindow::setCurrentFile(const QString &fileName)
{
    currentFile = fileName;
    //  setWindowModified(false);

    QString shownName = currentFile;
    if (currentFile.isEmpty())
        shownName = "untitled.json";

    setWindowFilePath(shownName);
}

bool MainWindow::saveFile(const QString &fileName)
{
    //
    // open file
    //

    QFile file(fileName);
    if (!file.open(QFile::WriteOnly | QFile::Text)) {
        QMessageBox::warning(this, tr("Application"),
                             tr("Cannot write file %1:\n%2.")
                             .arg(QDir::toNativeSeparators(fileName),
                                  file.errorString()));
        return false;
    }


    //
    // create a json object, fill it in & then use a QJsonDocument
    // to write the contents of the object to the file in JSON format
    //

    QJsonObject json;
    theRockOutcropWidget->outputToJSON(json);
    QJsonDocument doc(json);
    file.write(doc.toJson());

    // close file
    file.close();

    // set current file
    setCurrentFile(fileName);

    return true;
}

void MainWindow::loadFile(const QString &fileName)
{
    //
    // open file
    //

    QFile file(fileName);
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        QMessageBox::warning(this, tr("Application"),
                             tr("Cannot read file %1:\n%2.")
                             .arg(QDir::toNativeSeparators(fileName), file.errorString()));
        return;
    }

    // validate the document
    // JsonValidator class already takes a model type param, add additional model types as required

    /*
    JsonValidator *jval = new JsonValidator();
    jval->validate(this, BIM, fileName);
*/

    // place contents of file into json object
    QString val;
    val=file.readAll();
    QJsonDocument doc = QJsonDocument::fromJson(val.toUtf8());
    QJsonObject jsonObj = doc.object();

    // close file
    file.close();

    // given the json object, create the C++ objects
    if ( ! (currentFile.isNull() || currentFile.isEmpty()) ) {
        // theRockOutcropWidget->clear(); no method
    }
    theRockOutcropWidget->inputFromJSON(jsonObj);

    setCurrentFile(fileName);
}

void MainWindow::version()
{
    QString versionText("Version 1.1.1");
    QMessageBox msgBox;
    QSpacerItem *theSpacer = new QSpacerItem(700, 0, QSizePolicy::Minimum, QSizePolicy::Expanding);
    msgBox.setText(versionText);
    QGridLayout *layout = (QGridLayout*)msgBox.layout();
    layout->addItem(theSpacer, layout->rowCount(),0,1,layout->columnCount());
    msgBox.exec();
}

void MainWindow::cite()
{
    QString citeText("Charles Wang, Frank McKenna, Peter Mackenzie-Helnwein, Wael Elhaddad, Adam Zsarnoczay, Michael Gardner, & Pedro Arduino. (2019, September 27). NHERI-SimCenter/s3hark: Release v1.1.1 (Version v1.1.1). Zenodo. http://doi.org/10.5281/zenodo.3463594");
    QMessageBox msgBox;
    QSpacerItem *theSpacer = new QSpacerItem(700, 0, QSizePolicy::Minimum, QSizePolicy::Expanding);
    msgBox.setText(citeText);
    QGridLayout *layout = (QGridLayout*)msgBox.layout();
    layout->addItem(theSpacer, layout->rowCount(),0,1,layout->columnCount());
    msgBox.exec();
}


void MainWindow::about()
{
    QString aboutText("A SimCenter Tool For Site Response Analysis");
    QMessageBox msgBox;
    QSpacerItem *theSpacer = new QSpacerItem(700, 0, QSizePolicy::Minimum, QSizePolicy::Expanding);
    msgBox.setText(aboutText);
    QGridLayout *layout = (QGridLayout*)msgBox.layout();
    layout->addItem(theSpacer, layout->rowCount(),0,1,layout->columnCount());
    msgBox.exec();
}


void MainWindow::submitFeedback()
{
    QString feedbackURL("https://docs.google.com/forms/d/e/1FAIpQLSfh20kBxDmvmHgz9uFwhkospGLCeazZzL770A2GuYZ2KgBZBA/viewform");
    QDesktopServices::openUrl(QUrl(feedbackURL, QUrl::TolerantMode));
    //QDesktopServices::openUrl(QUrl("https://www.designsafe-ci.org/help/new-ticket/", QUrl::TolerantMode));
}

void MainWindow::manual()
{
    QString manualURL("https://www.designsafe-ci.org/data/browser/public/designsafe.storage.community//SimCenter/Software/s3hark");
    QDesktopServices::openUrl(QUrl(manualURL, QUrl::TolerantMode));
    //QDesktopServices::openUrl(QUrl("https://www.designsafe-ci.org/help/new-ticket/", QUrl::TolerantMode));
}

void MainWindow::submitFeatureRequest()
{
    QString featureRequestURL("https://docs.google.com/forms/d/e/1FAIpQLScTLkSwDjPNzH8wx8KxkyhoIT7AI9KZ16Wg9TuW1GOhSYFOag/viewform");
    QDesktopServices::openUrl(QUrl(featureRequestURL, QUrl::TolerantMode));
    //QDesktopServices::openUrl(QUrl("https://www.designsafe-ci.org/help/new-ticket/", QUrl::TolerantMode));
}

void MainWindow::copyright()
{
  QMessageBox msgBox;
  QString copyrightText = QString("\
                          <p>\
                          The source code is licensed under a BSD 2-Clause License:<p>\
                          \"Copyright (c) 2017-2019, The Regents of the University of California (Regents).\"\
                          All rights reserved.<p>\
                          <p>\
                          Redistribution and use in source and binary forms, with or without \
                          modification, are permitted provided that the following conditions are met:\
                          <p>\
                          1. Redistributions of source code must retain the above copyright notice, this\
                          list of conditions and the following disclaimer.\
                          \
                          \
                          2. Redistributions in binary form must reproduce the above copyright notice,\
                          this list of conditions and the following disclaimer in the documentation\
                          and/or other materials provided with the distribution.\
                          <p>\
                          THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS \"AS IS\" AND\
                          ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED\
                          WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE\
                          DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR\
                          ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES\
                          (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;\
                          LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND\
          ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT\
          (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS\
          SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.\
          <p>\
          The views and conclusions contained in the software and documentation are those\
          of the authors and should not be interpreted as representing official policies,\
          either expressed or implied, of the FreeBSD Project.\
          <p>\
          REGENTS SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT LIMITED TO, \
          THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.\
          THE SOFTWARE AND ACCOMPANYING DOCUMENTATION, IF ANY, PROVIDED HEREUNDER IS \
          PROVIDED \"AS IS\". REGENTS HAS NO OBLIGATION TO PROVIDE MAINTENANCE, SUPPORT,\
          UPDATES, ENHANCEMENTS, OR MODIFICATIONS.\
          <p>\
          ------------------------------------------------------------------------------------\
          <p>\
          The compiled binary form of this application is licensed under a GPL Version 3 license.\
          The licenses are as published by the Free Software Foundation and appearing in the LICENSE file\
          included in the packaging of this application. \
          <p>\
          ------------------------------------------------------------------------------------\
          <p>\
          This software makes use of the QT packages (unmodified): core, gui, widgets and network\
                                                                   <p>\
                                                                   QT is copyright \"The Qt Company Ltd&quot; and licensed under the GNU Lesser General \
                                                                   Public License (version 3) which references the GNU General Public License (version 3)\
    <p>\
    The licenses are as published by the Free Software Foundation and appearing in the LICENSE file\
    included in the packaging of this application. \
    <p>\
    ");

  QSpacerItem *theSpacer = new QSpacerItem(700, 0, QSizePolicy::Minimum, QSizePolicy::Expanding);
  msgBox.setText(copyrightText);
  QGridLayout *layout = (QGridLayout*)msgBox.layout();
  layout->addItem(theSpacer, layout->rowCount(),0,1,layout->columnCount());
  msgBox.exec();

}
