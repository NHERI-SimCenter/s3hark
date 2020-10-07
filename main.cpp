#include "UI/MainWindow.h"
#include <QApplication>
#include <QChar>
#include <QFile>
#include <QDir>
#include <QDebug>
#include <QStandardPaths>
#include <QTime>
#include <QSplashScreen>
#include <QDesktopWidget>
#include <GoogleAnalytics.h>

static QString logFilePath;
static bool logToFile = false;

void customMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    QHash<QtMsgType, QString> msgLevelHash({{QtDebugMsg, "Debug"}, {QtInfoMsg, "Info"}, {QtWarningMsg, "Warning"}, {QtCriticalMsg, "Critical"}, {QtFatalMsg, "Fatal"}});
    QByteArray localMsg = msg.toLocal8Bit();
    QTime time = QTime::currentTime();
    QString formattedTime = time.toString("hh:mm:ss.zzz");
    QByteArray formattedTimeMsg = formattedTime.toLocal8Bit();
    QString logLevelName = msgLevelHash[type];
    QByteArray logLevelMsg = logLevelName.toLocal8Bit();

    if (logToFile) {
        QString txt = QString("%1 %2: %3 (%4)").arg(formattedTime, logLevelName, msg,  context.file);
        QFile outFile(logFilePath);
        outFile.open(QIODevice::WriteOnly | QIODevice::Append);
        QTextStream ts(&outFile);
        ts << txt << endl;
        outFile.close();
    } else {
        fprintf(stderr, "%s %s: %s (%s:%u, %s)\n", formattedTimeMsg.constData(), logLevelMsg.constData(), localMsg.constData(), context.file, context.line, context.function);
        fflush(stderr);
    }

    if (type == QtFatalMsg)
        abort();
}

int main(int argc, char *argv[])
{
    GoogleAnalytics::SetTrackingId("UA-162363329-1");
    GoogleAnalytics::StartSession();
    GoogleAnalytics::ReportStart();
    //
    //Setting Core Application Name, Organization, Version and Google Analytics Tracking Id
    //

    QCoreApplication::setApplicationName("s3hark");
    QCoreApplication::setOrganizationName("SimCenter");
    QCoreApplication::setApplicationVersion("1.5");

    //
    // set up logging of output messages for user debugging
    //

    // create dir for log file if none yet exists
    logFilePath = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)
            + QDir::separator() + QCoreApplication::applicationName();

    QDir dirWork(logFilePath);
    if (!dirWork.exists())
        if (!dirWork.mkpath(logFilePath)) {
            qDebug() << "s3hark - main - could not create directory for log file";
        }

    // remove old log file as want to append to an empty file
    logFilePath = logFilePath + QDir::separator() + QString("debug.log");
    QFile debugFile(logFilePath);
    debugFile.remove();

    QByteArray envVar = qgetenv("QTDIR");       //  check if the app is run in Qt Creator

    if (envVar.isEmpty())
        logToFile = true;

    qInstallMessageHandler(customMessageOutput);

    qDebug() << "s3hark logFile: " << logFilePath;

    //
    // window scaling
    //

    QGuiApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

    //
    // regular Qt startup
    //
    QApplication a(argc, argv);

    // with a splash
    QSplashScreen *splash = new QSplashScreen;
    splash->setPixmap(QPixmap(":/resources/images/start.png"));
    splash->showMessage("Loading...   ", Qt::AlignRight);
    splash->show();
    QDateTime n=QDateTime::currentDateTime();
    /*
     * // block the whole thing
    QDateTime now;
    do{
        now=QDateTime::currentDateTime();
    } while (n.secsTo(now)<=1);
    */

    //sleep
    for(int i=0;i<500;i++)
    {
        splash->repaint();
    }


    MainWindow m;
    m.setWindowTitle(QString("S") + QChar(0x00B3) + "HARK - Site-Specific Seismic Hazard Analysis & Research Kit" );
    m.show();

    m.move ((QApplication::desktop()->width() - m.width())/2,(QApplication::desktop()->height() - m.height())/2);
    //splash->finish(&m);
    delete splash;

    //
    // show the main window, set styles & start the event loop
    //

#ifdef Q_OS_WIN
    QFile file(":/styleCommon/stylesheetWIN.qss");
#endif

#ifdef Q_OS_MACOS
    QFile file(":/styleCommon/stylesheetMAC.qss");
#endif

#ifdef Q_OS_LINUX
    QFile file(":/styleCommon/stylesheetMAC.qss");
#endif

    if(file.open(QFile::ReadOnly)) {
        a.setStyleSheet(file.readAll());
        file.close();
    } else {
        qDebug() << "could not open stylesheet";
    }


    int res = a.exec();

    GoogleAnalytics::EndSession();

    return res;

}
